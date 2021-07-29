#include "../utils/utils.h"
#include "hpack.h"
#include "tables.h"
#include "huffman.h"
#include "../utils/result.h"
#include "../utils/net.h"

// adapted from https://cs.opensource.google/go/x/net/+/c6fcb2db:http2/hpack/hpack.go

enum IndexType {
	True,
	False,
	Never
};

Result<HeaderField> getIndexedField(uint64_t index) {
	if (index > 60) {
		return Result<HeaderField>::ofError("cannot get index with no dyn tab");
    }
    return Result<HeaderField>::ofValue(STATIC_TABLE_ENTRIES[index]);
}

// the string returned is owned by the caller
Result<shared_ptr<char>> readString(uint8_t* data, uint32_t& dataIdx, uint32_t& dataLenRem) {
	bool isHuff = (data[dataIdx]&128) != 0;
	Result<uint64_t> strLenResult = readVarInt(7, data, dataIdx, dataLenRem);
	if (!strLenResult.isOk()) {
		return Result<shared_ptr<char>>::ofError(strLenResult.error());
	}
	uint64_t strLen = strLenResult.value();
	if (!isHuff) {
		shared_ptr<char> s(new char[strLen+1], default_delete<char[]>());
		s.get()[strLen] = 0;
		strncpy(s.get(), (char*)(data+dataIdx), strLen);
		dataIdx += strLen;
		dataLenRem -= strLen;
		return Result<shared_ptr<char>>::ofValue(s);
	}
	uint32_t decodedLen = 0;
	Result<shared_ptr<char>> result = huffmanDecode(5555555, data+dataIdx, strLen, decodedLen);
	if (!result.isOk()) {
		return result;
	}
	dataIdx += strLen;
	dataLenRem -= strLen;
	return result;
}

Result<HeaderField> parseFieldIndexed(uint8_t* data, uint32_t& dataIdx, uint32_t& dataLenRem) {
	Result<uint64_t> nameIdxResult = readVarInt(7, data, dataIdx, dataLenRem);
	if (!nameIdxResult.isOk()) {
		return Result<HeaderField>::ofError(nameIdxResult.error());
	}
	return getIndexedField(nameIdxResult.value());
}

Result<HeaderField> parseFieldLiteral(uint8_t n, IndexType it, uint8_t* data, uint32_t& dataIdx, uint32_t& dataLenRem) {
	HeaderField hf = HeaderField();

	Result<uint64_t> nameIdxResult = readVarInt(n, data, dataIdx, dataLenRem);
	if (!nameIdxResult.isOk()) {
		return Result<HeaderField>::ofError(nameIdxResult.error());
	}
	uint64_t nameIdx = nameIdxResult.value();
	if (nameIdx > 0) {
		Result<HeaderField> indexedFieldResult = getIndexedField(nameIdx);
		if (!indexedFieldResult.isOk()) {
			return Result<HeaderField>::ofError(indexedFieldResult.error());
		}
		hf.name = indexedFieldResult.value().name;
	} else {
		Result<shared_ptr<char>> readResult = readString(data, dataIdx, dataLenRem);
		if (!readResult.isOk()) {
			return Result<HeaderField>::ofError(readResult.error());
		}
		hf.name = readResult.value();
	}

	Result<shared_ptr<char>> readResult = readString(data, dataIdx, dataLenRem);
	if (!readResult.isOk()) {
		return Result<HeaderField>::ofError(readResult.error());
	}
	hf.value = readResult.value();

	if (it == Never) {
		hf.sensitive = true;
	}

	return Result<HeaderField>::ofValue(hf);
}

uint32_t encodeHeaderFieldsLen(const vector<HeaderField> & fields) {
	uint32_t len = 0;
	for (const HeaderField & field : fields) {
		len++; // type byte
		char * name = field.name.get();
		uint32_t nameLen = strlen(name);
		uint32_t nameEncodedLen = HuffmanEncodeLength(reinterpret_cast<uint8_t*>(name), nameLen);

		if (nameEncodedLen < nameLen) {
			len += varIntSize(7, nameEncodedLen);
			len += nameEncodedLen;
		} else {
			len += varIntSize(7, nameLen);
			len += nameLen;
		}

		char * value = field.value.get();
		uint32_t valueLen = strlen(value);
		uint32_t valueEncodedLen = HuffmanEncodeLength(reinterpret_cast<uint8_t*>(value), valueLen);
		if (valueEncodedLen < valueLen) {
			len += varIntSize(7, valueEncodedLen);
			len += valueEncodedLen;
		} else {
			len += varIntSize(7, valueLen);
			len += valueLen;
		}
	}
	return len;
}

uint8_t* encodeHeaderFields(const vector<HeaderField> & fields, uint32_t &outLen) {
	outLen = encodeHeaderFieldsLen(fields);
	uint8_t* encoded = new uint8_t[outLen];

	uint32_t encodeIdx = 0;
	for (const HeaderField & field : fields) {
		encoded[encodeIdx++] = 0x10; // never indexed, sensitive
		char * name = field.name.get();
		uint32_t nameLen = strlen(name);
		uint32_t nameEncodedLen = HuffmanEncodeLength(reinterpret_cast<uint8_t*>(name), nameLen);
		if (nameEncodedLen < nameLen) {
			uint32_t startIdx = encodeIdx;
			encodeIdx += writeVarInt(7, nameEncodedLen, encoded+encodeIdx);
			shared_ptr<uint8_t> nameEncoded = huffmanEncode(reinterpret_cast<uint8_t*>(name), nameLen, nameEncodedLen);
			memcpy(encoded + encodeIdx, nameEncoded.get(), nameEncodedLen);
			encodeIdx += nameEncodedLen;
			encoded[startIdx] |= 0x80;
		} else {
			encodeIdx += writeVarInt(7, nameLen, encoded+encodeIdx);
			memcpy(encoded+encodeIdx, name, nameLen);
			encodeIdx += nameLen;
		}
		

		char * value = field.value.get();
		uint32_t valueLen = strlen(value);
		uint32_t valueEncodedLen = HuffmanEncodeLength(reinterpret_cast<uint8_t*>(value), valueLen);
		if (valueEncodedLen < valueLen) {
			uint32_t startIdx = encodeIdx;
			encodeIdx += writeVarInt(7, valueEncodedLen, encoded+encodeIdx);
			shared_ptr<uint8_t> valueEncoded = huffmanEncode(reinterpret_cast<uint8_t*>(value), valueLen, valueEncodedLen);
			memcpy(encoded + encodeIdx, valueEncoded.get(), valueEncodedLen);
			encodeIdx += valueEncodedLen;
			encoded[startIdx] |= 0x80;
		} else {
			encodeIdx += writeVarInt(7, valueLen, encoded+encodeIdx);
			memcpy(encoded+encodeIdx, value, valueLen);
			encodeIdx += valueLen;
		}
	}	

	return encoded;
}

Result<vector<HeaderField>> decodeHeaderFields(uint8_t* data, uint32_t dataLength) {
	vector<HeaderField> headerFields = vector<HeaderField>();

	uint32_t lengthRem = dataLength;
    uint32_t dataIdx = 0;
	while (lengthRem > 0) {
		uint8_t bitPrefix = data[dataIdx];
		Result<HeaderField> hfResult;
	    if ((bitPrefix&128) != 0) {
	        hfResult = parseFieldIndexed(data, dataIdx, lengthRem);
	    } else if ((bitPrefix&192) == 64) {
	        hfResult = parseFieldLiteral(6, IndexType::True, data, dataIdx, lengthRem);
	    } else if ((bitPrefix&240) == 0) {
	        hfResult = parseFieldLiteral(4, IndexType::False, data, dataIdx, lengthRem);
	    } else if ((bitPrefix&240) == 16) {
	        hfResult = parseFieldLiteral(4, IndexType::Never, data, dataIdx, lengthRem);;
	    } else if ((bitPrefix&224) == 32) {
	        Result<uint64_t> updateResult = readVarInt(5, data, dataIdx, lengthRem);
	        if (!updateResult.isOk()) {
	        	return Result<vector<HeaderField>>::ofError(updateResult.error());
	        }
	        continue;
	    } else {
	    	return Result<vector<HeaderField>>::ofError("idk what kinda header field this is");
	        break;
	    }
	    if (!hfResult.isOk()) {
	    	return Result<vector<HeaderField>>::ofError(hfResult.error());
	    }
        headerFields.push_back(hfResult.value());
	}	

	return Result<vector<HeaderField>>::ofValue(headerFields);
}

// #ifndef ARDUINO
// int main() {
// 	vector<HeaderField> fieldsToEncode;
// 	fieldsToEncode.push_back(HeaderField("one", "two"));
// 	fieldsToEncode.push_back(HeaderField("three", "four"));
// 	uint32_t encodedLen = 0;
// 	uint8_t* encoded = encodeHeaderFields(fieldsToEncode, encodedLen);

// 	// uint8_t testData[] = {
// 	// 	32, 131, 134, 5, 152, 98, 187, 14, 147, 174, 58, 204, 191, 112, 175, 180, 241, 157, 59, 139, 103, 115, 16, 172, 110, 72, 211, 106, 63, 1, 140, 11, 226, 92, 46, 60, 184, 151, 102, 227, 192, 120, 31, 15, 16, 139, 29, 117, 208, 98, 13, 38, 61, 76, 77, 101, 100, 15, 43, 138, 154, 202, 200, 180, 199, 96, 43, 178, 242, 224, 0, 2, 116, 101, 134, 77, 131, 53, 5, 177, 31, 0, 137, 154, 202, 200, 178, 77, 73, 79, 106, 127, 134, 105, 247, 220, 11, 46, 91
// 	// };
// 	// Result<vector<HeaderField>> fieldsResult = decodeHeaderFields(testData, sizeof(testData));
// 	Result<vector<HeaderField>> fieldsResult = decodeHeaderFields(encoded, encodedLen);
// 	if (!fieldsResult.isOk()) {
// 		debugPrint(fieldsResult.error());
// 		return 1;
// 	}
// 	for (HeaderField hf : fieldsResult.value()) {
// 	    debugPrint(hf.name);
//         debugPrint(hf.value);
// 	}
// }
// #endif
