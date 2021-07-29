#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <HardwareSerial.h>
#include <vector> // do this after Arduino to redefine some macros
#define debugPrint(val) {\
	Serial.println(val);\
	Serial.flush();\
}
#define exit(code) delay(1)
#else
#include <iostream>
using namespace std;
#define debugPrint(val) cout << (val) << endl
#endif

int freeMemory();
