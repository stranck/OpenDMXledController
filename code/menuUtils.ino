#include <LiquidCrystal_I2C.h>

void backMenu(){
	//if(currentExecMode > 0)
	doneMode();
}

#define NAME_LINE "Open DMXled ctrl"
#define CREDIT_LINE "by Stranck"
#define VERSION_LINE "Version1.0"
#define HOWMANY_MODES 3
#define MMENU_FULL_CYCLE 600
const RunMode mmModes[HOWMANY_MODES] = {mode_dmx, mode_rgb, mode_auto/*, mode_testScreen*/};
uint16_t mmenuCounter = MMENU_FULL_CYCLE; 
uint8_t selectedMode, originalMode;
void mainMenu(){
	if(modeUpdated){
		wakeUpDisplay();
		originalMode = selectedMode = getModeIndex(mmModes, HOWMANY_MODES, allModes[currentExecMode + 1]);
		printMMenuSecondLine();
		mmenuCounter = 0;
		modeUpdated = false;
	}
	printMMenuFirstLine();
}
String mmenu1Txt = "";
void printMMenuFirstLine(){
	if(mmenuCounter == 400)
		mmenu1Txt = centerText(VERSION_LINE);
	if(mmenuCounter == 200)
		mmenu1Txt = centerText(CREDIT_LINE);
	else if(mmenuCounter == 0){
		mmenu1Txt = centerText(NAME_LINE);
		mmenuCounter = MMENU_FULL_CYCLE;
	}
	if(mmenu1Txt != ""){
		lcd.setCursor(0, 0);
		lcd.print(mmenu1Txt);
		mmenu1Txt = "";
	}
	mmenuCounter--;
}
String mmenu2Txt;
void printMMenuSecondLine(){
	if(selectedMode == 255)
		selectedMode = HOWMANY_MODES - 1;
	else if(selectedMode == HOWMANY_MODES)
		selectedMode = 0;

	mmenu2Txt = centerText(mmModes[selectedMode].displayName);
	mmenu2Txt[0] = '<';
	mmenu2Txt[15] = '>';

	if(originalMode == selectedMode){
		uint8_t i;
		for(i = 1; i < 16; i++){
			//Serial.println(String(i) + " " + mmenu2Txt[i]);
			if(mmenu2Txt[i] != ' '){
				mmenu2Txt[i - 1] = 126;
				break;
			}
		}
		for(i = 14; i != 0xff; i--)
			if(mmenu2Txt[i] != ' '){
				mmenu2Txt[i + 1] = 127;
				break;
			}
	}
	lcd.setCursor(0, 1);
	lcd.print(mmenu2Txt);
}
void menuMMenuPressed(){
	runMode(mmModes[originalMode]);
}
void leftMMenuPressed(){
	selectedMode--;
	printMMenuSecondLine();
}
void rightMMenuPressed(){
	selectedMode++;
	printMMenuSecondLine();
}
void enterMMenuPressed(){
	setMode(mmModes[selectedMode].id);
	runMode(mmModes[selectedMode]);
}

#define SVM_START_FAST 101
void (*reloadScreen)(), (*confirmSVM)(), (*exitSVM)(), (*exec)();
uint16_t originalValue, maxCap, minCap, calcValue, loopBackValue = 0xffff;
int8_t pressing;
uint8_t *value, *nextByte, svmCounter;
bool bit16;
void initSelectValueMenu(uint8_t *v, void (*rs)(), uint16_t maxC, uint16_t minC, void (*e)(), void (*m)(), void (*f)()){
	value = v;
	nextByte = value + 1;
	originalValue = *v;
	reloadScreen = rs;
	maxCap = maxC;
	minCap = minC;
	bit16 = maxCap > 0xff || minCap > 0xff;
	confirmSVM = e;
	exitSVM = m;
	exec = f;
	svmCounter = 0;
	enableBackLight();
	autoLCDturnOff = false;
	lcd.blink();
	reloadScreen();
	Serial.println("Editing value: " + String(*v) + ";" + String(*nextByte));
	runMode(mode_selectValueMenu);
}
void selectValueMenu(){
	if(pressing && svmCounter < 0xff)
		svmCounter++;
	if(pressing && (svmCounter == 1 || svmCounter > SVM_START_FAST)){
		//updateValue(pressing);
		*value += pressing;
		if(bit16 && ((pressing > 0 && *value == 0) || (pressing < 0 && *value == 255)))
			*nextByte += pressing;

		calcValue = *value;
		if(bit16)
			calcValue |= *nextByte << 8;
		/*Serial.println(String(calcValue) + " MIN:"
			+ String(calcValue < minCap) + " " + String(minCap) + " MAX:"
			+ String(calcValue > maxCap) + " " + String(maxCap));*/
		/*if(calcValue < minCap || calcValue > maxCap){
			updateValue(-pressing);
			svmReleased();
		} else reloadScreen();*/
		if(calcValue < minCap)
			loopBackValue = maxCap;
		else if(calcValue > maxCap)
			loopBackValue = minCap;
		if(loopBackValue != 0xffff){
			*value = uint8_t(loopBackValue & 0xff);
			if(bit16)
				*nextByte = uint8_t((loopBackValue >> 8) & 0xff);
			loopBackValue = 0xffff;
		}
		reloadScreen();
	}
	if(exec != NULL)
		exec();
}
/*void updateValue(int8_t addValue){
	*value += addValue;
	if(bit16 && ((addValue > 0 && *value == 0) || (addValue < 0 && *value == 255)))
		*nextByte += addValue;
}*/
void menuSVMPressed(){
	Serial.println("Aborting");
	if(bit16){
		uint16_t *v16 = (uint16_t*) value;
		*v16 = originalValue;
	} else {
		*value = uint8_t(originalValue & 0xff);
	}
	if(exitSVM != NULL)
		exitSVM();
	lcd.noBlink();
	autoLCDturnOff = true;
	doneMode();
}
void enterSVMPressed(){
	Serial.println("Done!");
	if(confirmSVM != NULL)
		confirmSVM();
	lcd.noBlink();
	autoLCDturnOff = true;
	doneMode();
}
void leftSVMPressed(){
	pressing = -1;
	lcd.noBlink();
}
void rightSVMPressed(){
	pressing = 1;
	lcd.noBlink();
}
void svmReleased(){
	pressing = 0;
	svmCounter = 0;
	lcd.blink();
}

uint16_t testScreenCounter;
int8_t testScreenOperator;
void testScreenChars(){
	if(modeUpdated){
		testScreenCounter = 0;
		testScreenOperator = 1;
		lcd.noBlink();
		wakeUpDisplay();
		printTestScreen();
		modeUpdated = false;
	}
}
void printTestScreen(){
	Serial.println(String(testScreenCounter) + "\tOP:" + String(testScreenOperator));
	char line1[17], line2[17], *buffer;
	line1[16] = line2[16] = 0x00;
	buffer = line1;
	for(uint8_t i = 0; i < 32; i++){
		testScreenCounter += testScreenOperator;
		if(i == 16)
			buffer = line2;
		buffer[i % 16] = testScreenCounter;
	}
	lcd.setCursor(0, 0);
	lcd.print(line1);
	lcd.setCursor(0, 1);
	lcd.print(line2);
}
void testScreenActionPlus(){
	testScreenOperator = 1;
	printTestScreen();
}
void testScreenActionMinus(){
	testScreenOperator = -1;
	printTestScreen();
}