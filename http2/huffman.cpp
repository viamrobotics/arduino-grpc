#include "../utils/utils.h"
#include <stdint.h>
// #include <iostream>
#include "huffman.h"
#include "tables.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <HardwareSerial.h>
#endif

using namespace std;

// adapted from https://cs.opensource.google/go/x/net/+/c6fcb2db:http2/hpack/huffman.go;bpv=1;bpt=1

struct node {
	// children is non-nil for internal nodes
	node** children = nullptr;

	uint8_t codeLen  = 0;
	uint8_t sym      = 0;
};


node* lazyRootHuffmanNode = nullptr;

node* newInternalNode() {
	node* internalNode = new node();
	internalNode->children = new node*[256]{};
	return internalNode;
}

node* newLeafNode(uint8_t sym, uint8_t codeLen) {
	node* leafNode = new node();
	leafNode->sym = sym;
	leafNode->codeLen = codeLen;
	return leafNode;
}

void addDecoderNode(uint8_t sym, uint32_t code, uint8_t codeLen) {
	node* cur = lazyRootHuffmanNode;
	while (codeLen > 8) {
		codeLen -= 8;
		uint8_t i = static_cast<uint8_t>(code >> codeLen);
		if (cur->children[i] == nullptr) {
			cur->children[i] = newInternalNode();
		}
		cur = cur->children[i];
	}
	uint8_t shift = 8 - codeLen;
	uint32_t start = static_cast<uint8_t>(code << shift);
	uint32_t end = 1 << shift;
	for (uint32_t i = start; i < start + end; i++) {
		cur->children[i] = newLeafNode(sym, codeLen);
	}
}

void buildRootHuffmanNode() {
	if (lazyRootHuffmanNode != nullptr) {
		return;
	}
	lazyRootHuffmanNode = newInternalNode();
	for (uint32_t i = 0; i < 256; i++) {
		// cout << "buildRootHuffmanNode: " << i << endl;
		uint32_t code = HUFFMAN_CODES[i];
		uint8_t codeLength = HUFFMAN_CODE_LENGTH[i];
		// cout << "addDecoderNode: " << i << " " << code << " " << codeLength << endl;
		addDecoderNode(static_cast<uint8_t>(i), code, codeLength);
	}
}

node* getRootHuffmanNode() {
	buildRootHuffmanNode();
	return lazyRootHuffmanNode;
}

Result<shared_ptr<char>> huffmanDecode(uint32_t maxLen, uint8_t* v, uint32_t vLen, uint32_t& decodedLen, bool decode) {
	shared_ptr<char> decoded;
	if (decode) {
		decoded = shared_ptr<char>(new char[decodedLen+1], default_delete<char[]>());
		decoded.get()[decodedLen] = 0;
	}
	uint32_t decodeIdx = 0;
	node* rootHuffmanNode = getRootHuffmanNode();
	node* n = rootHuffmanNode;
	// cur is the bit buffer that has not been fed into n.
	// cbits is the number of low order bits in cur that are valid.
	// sbits is the number of bits of the symbol prefix being decoded.
	uint32_t cur = 0;
	uint8_t cbits = 0;
	uint8_t sbits = 0;
	for (uint32_t vIdx = 0; vIdx < vLen; vIdx++) {
		uint8_t b = v[vIdx];
		cur = (cur<<8) | static_cast<uint8_t>(b);
		cbits += 8;
		sbits += 8;
		while (cbits >= 8) {
			uint8_t idx = static_cast<uint8_t>(cur >> (cbits - 8));
			n = n->children[idx];
			if (n == nullptr) {
				return Result<shared_ptr<char>>::ofError("ErrInvalidHuffman1");
			}
			if (n->children == nullptr) {
				// if (maxLen != 0 && buf.Len() == maxLen) {
				// 	cout << "return ErrStringLength" << endl;
				// 	return
				// }
				// buf.WriteByte(n.sym)
				// cout << n->sym;
				if (decode) {
					decoded.get()[decodeIdx] = n->sym;					
				}
				decodeIdx++;
				cbits -= n->codeLen;
				n = rootHuffmanNode;
				sbits = cbits;
			} else {
				cbits -= 8;
			}
		}
	}
	while (cbits > 0) {
		n = n->children[static_cast<uint8_t>(cur<<(8-cbits))];
		if (n == nullptr) {
			return Result<shared_ptr<char>>::ofError("ErrInvalidHuffman2");
		}
		if (n->children != nullptr || n->codeLen > cbits) {
			break;
		}
		// if maxLen != 0 && buf.Len() == maxLen {
		// 	cout << "return ErrStringLength" << endl;
		// 	return;
		// }
		// buf.WriteByte(n->sym)
		// cout << n->sym;
		if (decode) {
			decoded.get()[decodeIdx] = n->sym;					
		}
		decodeIdx++;
		cbits -= n->codeLen;
		n = rootHuffmanNode;
		sbits = cbits;
	}
	if (sbits > 7) {
		// Either there was an incomplete symbol, or overlong padding.
		// Both are decoding errors per RFC 7541 section 5.2.
		return Result<shared_ptr<char>>::ofError("ErrInvalidHuffman3");
	}
	{
		uint32_t mask = static_cast<uint32_t>((1<<cbits) - 1);
		if ((cur&mask) != mask) {
			// Trailing bits must be a prefix of EOS per RFC 7541 section 5.2.
			return Result<shared_ptr<char>>::ofError("ErrInvalidHuffman4");
		}
	}

	decodedLen = decodeIdx;
	return Result<shared_ptr<char>>::ofValue(decoded);
}

// TODO(erd): this should do errors properly
Result<shared_ptr<char>> huffmanDecode(uint32_t maxLen, uint8_t* v, uint32_t vLen, uint32_t& decodedLen) {
	huffmanDecode(maxLen, v, vLen, decodedLen, false);
	return huffmanDecode(maxLen, v, vLen, decodedLen, true);
}

uint64_t HuffmanEncodeLength(uint8_t* s, uint32_t sLen) {
	uint64_t n = 0;
	for (uint32_t i = 0; i < sLen; i++) {
		n += static_cast<uint64_t>(HUFFMAN_CODE_LENGTH[s[i]]);
	}
	return (n + 7) / 8;
}


uint8_t appendByteToHuffmanCode(shared_ptr<uint8_t> encoded, uint8_t rembits, uint8_t c, uint32_t& encodeIdx, bool encode) {
	uint32_t code = HUFFMAN_CODES[c];
	uint8_t nbits = HUFFMAN_CODE_LENGTH[c];

	while (true) {
		if (rembits > nbits) {
			uint8_t t = static_cast<uint8_t>(code << (rembits - nbits));
			if (encode) {
				encoded.get()[encodeIdx-1] |= t;
			}
			rembits -= nbits;
			break;
		}

		uint8_t t = static_cast<uint8_t>(code >> (nbits - rembits));
		if (encode) {
			encoded.get()[encodeIdx-1] |= t;			
		}

		nbits -= rembits;
		rembits = 8;

		if (nbits == 0) {
			break;
		}

		if (encode) {
			encoded.get()[encodeIdx] = 0;
		}
		encodeIdx++;
	}

	return rembits;
}


shared_ptr<uint8_t> huffmanEncode(uint8_t* s, uint32_t sLen, uint32_t& encodedLen, bool encode) {
	shared_ptr<uint8_t> encoded;
	if (encode) {
		encoded = shared_ptr<uint8_t>(new uint8_t[encodedLen], default_delete<uint8_t[]>());
	}
	uint32_t encodeIdx = 0;
	uint8_t rembits = 8;

	for (uint32_t i = 0; i < sLen; i++) {
		if (rembits == 8) {
			if (encode) {
				encoded.get()[encodeIdx] = 0;
			}
			encodeIdx++;
		}
		rembits = appendByteToHuffmanCode(encoded, rembits, s[i], encodeIdx, encode);
	}

	if (rembits < 8) {
		// special EOS symbol
		uint32_t code = 0x3fffffff;
		uint8_t nbits = 30;

		uint8_t t = static_cast<uint8_t>(code >> (nbits - rembits));
		if (encode) {
			encoded.get()[encodeIdx-1] |= t;			
		}
	}

	encodedLen = encodeIdx;
	return encoded;
}

shared_ptr<uint8_t> huffmanEncode(uint8_t* s, uint32_t sLen, uint32_t& encodedLen) {
	huffmanEncode(s, sLen, encodedLen, false);
	return huffmanEncode(s, sLen, encodedLen, true);
}


// #ifndef ARDUINO
// int main() {
// 	buildRootHuffmanNode();
// 	uint8_t testStr[] = {0x94, 0xe7, 0x82, 0x1d, 0xd7, 0xf2, 0xe6, 0xc7, 0xb3, 0x35, 0xdf, 0xdf, 0xcd, 0x5b, 0x39, 0x60, 0xd5, 0xaf, 0x27, 0x08, 0x7f, 0x36, 0x72, 0xc1, 0xab, 0x27, 0x0f, 0xb5, 0x29, 0x1f, 0x95, 0x87, 0x31, 0x60, 0x65, 0xc0, 0x03, 0xed, 0x4e, 0xe5, 0xb1, 0x06, 0x3d, 0x50, 0x07};
// 	uint32_t decodedLen = 0;
// 	shared_ptr<char> decoded = huffmanDecode(0, testStr, 45, decodedLen).value(); // foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1
// 	cout << decoded << endl;
// 	uint32_t encodedLen = 0;
// 	shared_ptr<uint8_t> encoded = huffmanEncode((uint8_t*)(decoded.get()), decodedLen, encodedLen);
// 	uint32_t decodedLen2 = 0;
// 	shared_ptr<char> decoded2 = huffmanDecode(0, encoded.get(), encodedLen, decodedLen2).value(); // foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1
// 	cout << decoded2 << endl;

// 	uint8_t testStr2[] = {98, 187, 14, 147, 174, 58, 204, 191, 112, 175, 180, 241, 157, 59, 139, 103, 115, 16, 172, 110, 72, 211, 106, 63};
// 	uint32_t decodedLen3 = 0;
// 	shared_ptr<char> decoded3 = huffmanDecode(0, testStr2, sizeof(testStr2), decodedLen3).value();
// 	cout << decoded3 << endl;

// 	uint8_t testStr3[] = {29, 117, 208, 98, 13, 38, 61, 76, 77, 101, 100};
// 	uint32_t decodedLen4 = 0;
// 	shared_ptr<char> decoded4 = huffmanDecode(0, testStr3, sizeof(testStr3), decodedLen4).value();
// 	cout << decoded4 << endl;
// }
// #endif
