#include <LiquidCrystal_I2C.h>
#include "Conceptinetics.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DMX_SLAVE_CHANNELS 4
#define RXEN_PIN 2
DMX_Slave dmx_slave(DMX_SLAVE_CHANNELS, RXEN_PIN);

#define SAVE_ADDRESS 69
#define MODE_MENUS 'u'
#define MODE_DEBUG '?'
#define MODE_DMX 'd'
#define MODE_RGB 'r'
#define MODE_AUTO 'a'
#define MODE_TESTSCREEN 't'
#define MODE_AUTO_RGB 'r'
#define MODE_AUTO_RYGCBM 'R'
#define MODE_AUTO_FADE 'f'
#define MODE_AUTO_FADEALL 'F'
#define MODE_AUTO_STROBO 's'
#define MODE_AUTO_AUTO 'a'
#define MODE_AUTO_OFF 'o'

#define PIN_GREEN 8
#define PIN_RED 9
#define PIN_BLUE 10

#define BUTTON_MENU 0
#define BUTTON_LEFT 1
#define BUTTON_RIGHT 2
#define BUTTON_ENTER 3
#define PIN_MENU 53
#define PIN_LEFT 51
#define PIN_RIGHT 49
#define PIN_ENTER 47

#define CHANNEL_RED 1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE 3
#define CHANNEL_STROBO 4

#define CYCLE_LENGTH_MS 10
#define EXEC_STACK_SIZE 4
#define CLEAR_LINE "                "
#define BACKLIGHT_DELAY 6000
#define DISPLAY_DELAY 6000
#define LCD_DELAY DISPLAY_DELAY + BACKLIGHT_DELAY

int16_t elaborateStrobe();
void setAllColors();
void checkButton();
void lcdBacklight();
void enableBackLight();
void lcdOn();
void loadSetting();
void backMenu();
struct RunMode {
	char id;
	void (*exec)();
	void (*menuPressed)();
	void (*leftPressed)();
	void (*rightPressed)();
	void (*enterPressed)();
	void (*menuReleased)();
	void (*leftReleased)();
	void (*rightReleased)();
	void (*enterReleased)();
	String displayName;
};
void mainMenu();
void leftMMenuPressed();
void rightMMenuPressed();
void enterMMenuPressed();
void menuMMenuPressed();
const RunMode mode_mainMenu = {
	MODE_MENUS,
	mainMenu,
	menuMMenuPressed,
	leftMMenuPressed,
	rightMMenuPressed,
	enterMMenuPressed,
	NULL,
	NULL,
	NULL,
	NULL,
	""
};
void selectValueMenu();
void menuSVMPressed();
void enterSVMPressed();
void leftSVMPressed();
void rightSVMPressed();
void svmReleased();
const RunMode mode_selectValueMenu = {
	MODE_MENUS,
	selectValueMenu,
	menuSVMPressed,
	leftSVMPressed,
	rightSVMPressed,
	enterSVMPressed,
	NULL,
	svmReleased,
	svmReleased,
	NULL,
	""
};
void dmxButtonAction();
void dmxMode();
const RunMode mode_dmx = {
	MODE_DMX,
	dmxMode,
	backMenu,
	dmxButtonAction,
	dmxButtonAction,
	dmxButtonAction,
	NULL,
	NULL,
	NULL,
	NULL,
	"DMX mode"
};
void rgbMode();
void rgbEnterPressed();
void rgbLeftPressed();
void rgbRightPressed();
const RunMode mode_rgb = {
	MODE_RGB,
	rgbMode,
	backMenu,
	rgbLeftPressed,
	rgbRightPressed,
	rgbEnterPressed,
	NULL,
	NULL,
	NULL,
	NULL,
	"RGB mode"
};
void autoModeFirstMenu();
void amfmEnterPressed();
void amfmLeftPressed();
void amfmRightPressed();
const RunMode mode_auto = {
	MODE_AUTO,
	autoModeFirstMenu,
	backMenu,
	amfmLeftPressed,
	amfmRightPressed,
	amfmEnterPressed,
	NULL,
	NULL,
	NULL,
	NULL,
	"Auto mode "
};
void autoModeSelectMenu();
void amsmEnterPressed();
void amsmLeftPressed();
void amsmRightPressed();
const RunMode mode_autoSelect = {
	MODE_MENUS,
	autoModeSelectMenu,
	backMenu,
	amsmLeftPressed,
	amsmRightPressed,
	amsmEnterPressed,
	NULL,
	NULL,
	NULL,
	NULL,
	""
};
void testScreenActionPlus();
void testScreenActionMinus();
void testScreenChars();
const RunMode mode_testScreen = {
	MODE_TESTSCREEN,
	testScreenChars,
	backMenu,
	testScreenActionMinus,
	testScreenActionPlus,
	backMenu,
	NULL,
	NULL,
	NULL,
	NULL,
	"TestScreen"
};
const RunMode mode_debug = {
	MODE_DEBUG,
	testScreenChars,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Debug mode"
};

String centerText(String text){
	uint8_t spaces = (16 - text.length()) / 2;
	char sp[spaces + 1];
	for(uint8_t i = 0; i < spaces; i++)
		sp[i] = ' ';
	sp[spaces] = 0x00;
	String s = String(sp);
	return s + text + s + " ";
}

void setup(){
	Serial.begin(9600);
	lcd.init();
	wakeUpDisplay();
	/*lcd.setCursor(0, 0);
	lcd.print("LED strip contr.");
	lcd.setCursor(0, 1);
	lcd.print("   by Stranck   ");*/
	loadSetting();
	for(int i = 0; i < 4; i++)
		delayCycle();
}

RunMode allModes[EXEC_STACK_SIZE];
RunMode execMode;
int8_t currentExecMode = -1;
bool modeUpdated = false;
void printModeName(){
	if(execMode.displayName != ""){
		lcd.setCursor(0, 0);
		lcd.print(centerText(execMode.displayName));
		Serial.println(centerText(execMode.displayName));
	}
}
void doneMode(){
	execMode = allModes[--currentExecMode];
	modeUpdated = true;
	printModeName();
	Serial.println("Current executing index: " + String(currentExecMode)/* + "\taddr: " + String(execMode[currentExecMode])*/);
}
void runMode(RunMode mode);
void runMode(RunMode mode){
	allModes[++currentExecMode] = mode;
	execMode = mode;
	modeUpdated = true;
	printModeName();
	Serial.println("Current executing index: " + String(currentExecMode)/* + "\taddr: " + String(execMode[currentExecMode])*/);
}


bool outputEnabled = true, strobeEnabled = false, autoLCDturnOff = true;
uint8_t redOut;
uint8_t greenOut;
uint8_t blueOut;
uint8_t strobeQuantity;
int16_t strobeCounter;
int buttonStatus[4] = {1, 1, 1, 1};

void loop(){
	execMode.exec();
	//Serial.println(String(strobeEnabled) + " " + String(strobeCounter) + " " + String(outputEnabled));
	if(strobeEnabled && --strobeCounter <= 0){
		//Serial.println("a");
		outputEnabled = !outputEnabled;
		strobeCounter = elaborateStrobe();
	}
	if(outputEnabled){
		analogWrite(PIN_RED, redOut);
		analogWrite(PIN_GREEN, greenOut);
		analogWrite(PIN_BLUE, blueOut);
	} else setAllColors(0);

	checkButton(PIN_MENU, &buttonStatus[BUTTON_MENU], execMode.menuPressed, execMode.menuReleased);
	checkButton(PIN_LEFT, &buttonStatus[BUTTON_LEFT], execMode.leftPressed, execMode.leftReleased);
	checkButton(PIN_RIGHT, &buttonStatus[BUTTON_RIGHT], execMode.rightPressed, execMode.rightReleased);
	checkButton(PIN_ENTER, &buttonStatus[BUTTON_ENTER], execMode.enterPressed, execMode.enterReleased);

	if(autoLCDturnOff) lcdBacklight();
	delayCycle();
	//delay(CYCLE_LENGTH_MS);
	//Serial.println(String(millis()));
}

unsigned long dcCurrent, dcOld = millis();
long dcWait;
void delayCycle(){
	dcCurrent = millis();
	dcWait = CYCLE_LENGTH_MS - (dcCurrent - dcOld);
	//Serial.println(String(dcWait));
	//Serial.print(String(millis()) + " ");
	if(dcWait > 0){
		//Serial.print(String(dcWait) + String(millis()) + " ");
		delay(dcWait);
		dcCurrent += dcWait;
		//Serial.println(String(millis()));
	}
	//else
	//	Serial.println("W: " + String(dcWait) + " " + String(dcCurrent) + " " + String(dcOld));
	dcOld = dcCurrent;
}

int cbValuee;
void (*cbExecc)();
bool enableInputs;
void checkButton(uint8_t button, int *status, void (*press)(), void(*rel)()){
	cbValuee = digitalRead(button);
	if(!cbValuee) enableBackLight();
	//Serial.println("Checking button: " + String(button) + " oldStatus: " + String(*status));
	if(*status != cbValuee){
		*status = cbValuee;
		if(cbValuee)
			cbExecc = rel;
		else
			cbExecc = press;
		//Serial.print("\tf: ");
		//Serial.print(cbExecc);
		if(cbExecc != NULL && enableInputs)
			cbExecc();
	}
	//Serial.println("\tDone!");
}

uint16_t backlightCounter = BACKLIGHT_DELAY;
void lcdBacklight(){
	if(backlightCounter){
		if(backlightCounter == 1)
			lcd.noDisplay();
		else if(backlightCounter == DISPLAY_DELAY){
			lcd.noBacklight();
			enableInputs = false;
		} else if(backlightCounter == LCD_DELAY){
			lcdOn();
			enableInputs = true;
		}
		backlightCounter--;
		//Serial.println(backlightCounter);
	}
}
void enableBackLight(){
	//Serial.println(String(backlightCounter) + " " + String(DISPLAY_DELAY));
	backlightCounter = LCD_DELAY;
	lcdOn();
}
void lcdOn(){
	lcd.backlight();
	lcd.display();
}
void wakeUpDisplay(){
	enableBackLight();
	autoLCDturnOff = true;
}

//void setInput(uint8_t r, uint8_t g, uint8_t b,/*uint8_t d,*/ uint8_t s);
/*DMX_FrameBuffer dmxBuffer = dmx_slave.getBuffer();
void OnFrameReceiveComplete(unsigned short channelsReceived){
	setInput(
		dmxBuffer[CHANNEL_RED],
		dmxBuffer[CHANNEL_GREEN],
		dmxBuffer[CHANNEL_BLUE],
		dmxBuffer[CHANNEL_STROBO]
	);
}*/
#define MIN_STROBE 4
#define STROBE_ON 2
int16_t oldElaboratedStrobe;
int16_t elaborateStrobe(){
	if(outputEnabled){
		return STROBE_ON;
	} else {
		return oldElaboratedStrobe = 255 + MIN_STROBE - strobeQuantity; //2 + (255 - strobeQuantity)
	}
}
bool sStatuss;
void setStrobe(uint8_t strobe){
	if(strobe != strobeQuantity){
		sStatuss = strobe > 16;
		strobeQuantity = strobe;
		if(sStatuss != strobeEnabled)
			outputEnabled = !sStatuss;
		strobeCounter = oldElaboratedStrobe - strobeCounter;
		strobeCounter = elaborateStrobe() - strobeCounter;
		//Serial.println(String())
		strobeEnabled = sStatuss;
	}
}

bool contains(char arr[], size_t size, char value){
	bool b = false;
	Serial.println("siz: " + String(size) + " char: " + value);
	for(uint8_t i = 0; i < size && !b; i++){
		Serial.println(arr[i]);
		b = arr[i] == value;
	}
	return b;
}

uint8_t getModeIndex(const RunMode modez[], uint8_t size, RunMode rm){
	while(--size != 0xff){
		/*Serial.println(String(size) + " rm:" + rm->id + " modez:" + modez[size].id);
		Serial.println ((long unsigned int) rm, HEX);
		Serial.println ((long unsigned int) &modez[size], HEX);*/
		if(modez[size].id == rm.id)
			return size;
	}
	Serial.println("Mode not found");
	return 0xff;
}

void setAllColors(int value){
	analogWrite(PIN_RED, value);
	analogWrite(PIN_GREEN, value);
	analogWrite(PIN_BLUE, value);
}