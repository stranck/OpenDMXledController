#include <EEPROM.h>
#include <Arduino.h>

template <class T> int EEPROM_write(int ee, const T& value){
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.update(ee++, *p++);
	return i;
}