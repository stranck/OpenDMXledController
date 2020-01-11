#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include "EEPROMAnythin.h"
#include "Conceptinetics.h"

struct SaveData {
	char selectedMode;

	uint16_t dmxAddress;

	uint8_t redValue;
	uint8_t greenValue;
	uint8_t blueValue;
	uint8_t stroboValue;

	char autoProgram;
	uint8_t autoSpeed;
	uint8_t autoChangeCycle;
};
SaveData savData;
//void (*printFristScreen)();

void saveSettings(){
	EEPROM_write(SAVE_ADDRESS, savData);
}

void loadSetting(){
	lcd.setCursor(0, 0);
	lcd.print(" Starting up... ");
	delay(500);
    readSav();
	verifySav();
	delay(350);
	startDmx();
	delay(400);
	loadPins();
	delay(300);
	testColors();
	lcd.setCursor(0, 1);
	lcd.print("     Done!!     ");
	delay(1000);
	setAllColors(0);
	loadMode();
	//printFristScreen();
}

void loadMode(){
	runMode(mode_mainMenu);
	Serial.println("Selected mode: " + String(int(savData.selectedMode)));
	switch(savData.selectedMode){
		case MODE_DMX: {
			runMode(mode_dmx);
			//printFristScreen = printDmxMenuAddr;
			break;
		}
		case MODE_RGB: {
			runMode(mode_rgb);
			break;
		}
		case MODE_AUTO: {
			runMode(mode_auto);
			break;
		}
		case MODE_DEBUG: {
			break;
		}
	}
}

void testColors(){
	lcd.setCursor(0, 1);
	lcd.print("Verify color:   ");

	pinMode(PIN_RED, OUTPUT);
	pinMode(PIN_GREEN, OUTPUT);
	pinMode(PIN_BLUE, OUTPUT);
	setAllColors(0);

	delay(250);
	testColor('R', PIN_RED);
	testColor('G', PIN_GREEN);
	testColor('B', PIN_BLUE);
	setAllColors(255);
}
void testColor(char c, int pin){
	lcd.setCursor(15, 1);
	lcd.print(c);
	analogWrite(pin, 255);
	delay(750);
	analogWrite(pin, 0);
}

void loadPins(){
	lcd.setCursor(0, 1);
	lcd.print("     Pinout     ");

	pinMode(PIN_MENU, INPUT_PULLUP);
	pinMode(PIN_LEFT, INPUT_PULLUP);
	pinMode(PIN_RIGHT, INPUT_PULLUP);
	pinMode(PIN_ENTER, INPUT_PULLUP);
	if(!digitalRead(PIN_MENU)){
		delay(200);
		savData.selectedMode = MODE_DEBUG;
		lcd.setCursor(0, 1);
		lcd.print("Enter debug mode");
		delay(1000);
	}
}

void OnFrameReceiveComplete(unsigned short channelsReceived);
void startDmx(){
	lcd.setCursor(0, 1);
	lcd.print("DMX shield start");

	dmx_slave.enable();
	dmx_slave.setStartAddress(getDmxAddress());
	dmx_slave.onReceiveComplete(OnFrameReceiveComplete);
}

void verifySav(){
	char modes[3] = {MODE_DMX, MODE_RGB, MODE_AUTO};
	char autoModes[7] = {MODE_AUTO_RGB, MODE_AUTO_RYGCBM, MODE_AUTO_FADE, MODE_AUTO_FADEALL, MODE_AUTO_STROBO, MODE_AUTO_AUTO, MODE_AUTO_OFF};
	bool changed = false;

	if(!contains(modes, (sizeof(modes) / sizeof(*modes)), getMode())){
		Serial.println("Invalid mode");
		savData.selectedMode = MODE_DMX;
		changed = true;
	}
	if(!contains(autoModes, (sizeof(autoModes) / sizeof(*autoModes)), getAutoProgram())){
		Serial.println("Invalid autoProgram");
		savData.autoProgram = MODE_AUTO_OFF;
		changed = true;
	}
	if(getDmxAddress() == 0 || getDmxAddress() > 512){
		Serial.println("Invalid dmx address");
		savData.dmxAddress = 1;
		changed = true;
	}
	/*if(savData.autoSpeed == 0){
		Serial.println("Invalid autoSpeed");
		savData.autoSpeed = 1;
		changed = true;
	}
	if(savData.autoChangeCycle == 0){
		Serial.println("Invalid autoChangeCycle");
		savData.autoChangeCycle = 1;
		changed = true;
	}*/

	if(changed)
		saveSettings();
}
void readSav(){
	lcd.setCursor(0, 1);
	lcd.print("Reading settings");

	EEPROM.get(SAVE_ADDRESS, savData);
}

void saveRGBSvalues(uint8_t red, uint8_t green, uint8_t blue, uint8_t strobo){
	savData.redValue = red;
	savData.greenValue = green;
	savData.blueValue = blue;
	savData.stroboValue = strobo;
	saveSettings();
}
void saveAutoSpeedValues(uint8_t speed, uint8_t cycles){
	savData.autoSpeed = speed;
	savData.autoChangeCycle = cycles;
	saveSettings();
}
void setMode(char mode){
	savData.selectedMode = mode;
	saveSettings();
}
void setAutoProgram(char program){
	savData.autoProgram = program;
	saveSettings();
}
void setDmxAddress(uint16_t address){
	savData.dmxAddress = address;
	dmx_slave.setStartAddress(address);
	saveSettings();
}

void readRGBSvalues(uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *strobo){
	*red = savData.redValue;
	*green = savData.greenValue;
	*blue = savData.blueValue;
	*strobo = savData.stroboValue;
}
void readAutoSpeedValues(uint8_t *speed, uint8_t *cycles){
	*speed = savData.autoSpeed;
	*cycles = savData.autoChangeCycle;
}
char getMode(){
	return savData.selectedMode;
}
char getAutoProgram(){
	return savData.autoProgram;
}
uint16_t getDmxAddress(){
	return savData.dmxAddress;
}