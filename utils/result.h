#pragma once

#include "../utils/utils.h"

template<class ValueType>
class Result {
private:
	ValueType _value;
	const char * _error = nullptr;
	bool _isOk = false;
public:
	static Result<ValueType> ofValue(ValueType value);
	static Result<ValueType> ofError(const char* error);
	bool isOk();
	ValueType value();
	const char * error();
};

template<class ValueType>
Result<ValueType> Result<ValueType>::ofValue(ValueType value) {
	Result result;
	result._isOk = true;
	result._value = value;
	return result;
}

template<class ValueType>
Result<ValueType> Result<ValueType>::ofError(const char* error) {
	Result result;
	result._isOk = false;
	result._error = error;
	return result;
}

template<class ValueType>
bool Result<ValueType>::isOk() {
	return this->_isOk;
}

template<class ValueType>
ValueType Result<ValueType>::value() {
	if (!this->_isOk) {
		debugPrint("value called when not ok");
		exit(1);
	}
	return this->_value;
}

template<class ValueType>
const char * Result<ValueType>::error() {
	if (!this->_isOk) {
		debugPrint("error called when not ok");
		exit(1);
	}
	return this->_error;
}

