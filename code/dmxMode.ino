#include <LiquidCrystal_I2C.h>
#include "Conceptinetics.h"

#define DMX_LOSE 50
#define SCREEN_ONOFF_DELAY 100

#define SCREEN_ONOFF_VALUE (DMX_LOSE + SCREEN_ONOFF_DELAY)

uint16_t dmxAddress;
uint8_t dmxRed, dmxGreen, dmxBlue, dmxStrobo;
uint8_t lastFrameCount;
bool dmxSig = true, oldDmxSig = !dmxSig;
bool backLight;

void dmxMode(){	
	dmxSig = lastFrameCount < DMX_LOSE;
	if(modeUpdated){
		oldDmxSig = !dmxSig;
		printDmxMenuAddr();
		outputDMXvalues();
		modeUpdated = false;
	}
	if(dmxSig) {
		outputDMXvalues();
	} else /*if(lastFrameCount == DMX_LOSE) {
		printDmxMenu("NO SIGNAL");
	} else*/ if(lastFrameCount > (SCREEN_ONOFF_VALUE)){
		backLight = !backLight;
		if(backLight)
			lcd.backlight();
		else
			lcd.noBacklight();
		lastFrameCount = DMX_LOSE;
	}
	if(dmxSig != oldDmxSig){
		oldDmxSig = dmxSig;
		dmxAddress = getDmxAddress();
		if(oldDmxSig){
			wakeUpDisplay();
			printDmxMenuAddr();
		} else {
			autoLCDturnOff = false;
			lcdOn();
			printDmxMenu("NO SIGNAL");
		}
	}
	//Serial.println(lastFrameCount);
	lastFrameCount++;
	//return !lastFrameCount && dmxSig;
}

void outputDMXvalues(){
	redOut = dmxRed;
	greenOut = dmxGreen;
	blueOut = dmxBlue;
	setStrobe(dmxStrobo);
}

DMX_FrameBuffer dmxBuffer = dmx_slave.getBuffer();
void OnFrameReceiveComplete(unsigned short channelsReceived){
	//Serial.println(String(dmxBuffer[3]) + " " + String(dmxBuffer[4]));
	dmxRed = dmxBuffer[CHANNEL_RED];
	dmxGreen = dmxBuffer[CHANNEL_GREEN];
	dmxBlue = dmxBuffer[CHANNEL_BLUE];
	dmxStrobo = dmxBuffer[CHANNEL_STROBO];
	lastFrameCount = 0;
}
/*void setInput(uint8_t r, uint8_t g, uint8_t b,/*uint8_t d,*//*uint8_t s){
	strobo = s;
	//r : 255 = red : dimmer
	/*red = (r * d) >> 8;
	green = (g * d) >> 8;
	blue = (b * d) >> 8;*//*
	red = r;
	green = g;
	blue = b;
	lastFrameCount = 0;
}*/

void printDmxMenu(String s){
	/*lcd.setCursor(0, 0);
	lcd.print("    DMX mode    ");*/
	//lcd.setCursor(8, 1);
	//lcd.print("       ");
	lcd.setCursor(0, 1);
	lcd.print(s + "       ");
	uint8_t pointer = 15;
	if(dmxAddress > 9) pointer--;
	if(dmxAddress > 99) pointer--;
	lcd.setCursor(pointer, 1);
	lcd.print(String(dmxAddress));
	lcd.setCursor(15, 1);
}
void printDmxMenuAddr(){
	printDmxMenu("Address:");
}

void dmxButtonAction(){
	initSelectValueMenu((uint8_t*) &dmxAddress, printDmxMenuAddr, 512, 1, confirmNewDMX, NULL, NULL);
}
void confirmNewDMX(){
	setDmxAddress(dmxAddress);
}