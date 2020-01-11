#include <LiquidCrystal_I2C.h>

struct AutoProgram {
	char id;
	void (*exec)();
	String name;
};
#define AP_PROGRAM_RGB 0
const AutoProgram ap_rgb = {
	MODE_AUTO_RGB,
	program_rgb,
	"RGB"
};
#define AP_PROGRAM_RYGCBM 1
const AutoProgram ap_rygcbm = {
	MODE_AUTO_RYGCBM,
	program_rygcbm,
	"RYGCBM"
};
#define AP_PROGRAM_FADE 2
const AutoProgram ap_fade_rgb = {
	MODE_AUTO_FADE,
	program_fade_rgb,
	"FadeRGB"
};
#define AP_PROGRAM_FADEALL 3
const AutoProgram ap_fade_rygcbm = {
	MODE_AUTO_FADEALL,
	program_fade_rygcbm,
	"FadeAll"
};
#define AP_PROGRAM_STROBE 4
const AutoProgram ap_strobe = {
	MODE_AUTO_STROBO,
	program_strobo,
	"Strobe"
};
#define AP_PROGRAM_AUTO 5
const AutoProgram ap_auto = {
	MODE_AUTO_AUTO,
	program_auto,
	"Auto"
};
#define AP_PROGRAM_OFF 6
const AutoProgram ap_off = {
	MODE_AUTO_OFF,
	program_off,
	"Off"
};
#define AUTOPROGRAM_NO 7
bool changedProg = true;
const AutoProgram autoProgs[AUTOPROGRAM_NO] = {ap_rgb, ap_rygcbm, ap_fade_rgb, ap_fade_rygcbm, ap_strobe, ap_auto, ap_off};
uint8_t getProgramIndex(char id){
	uint8_t size = AUTOPROGRAM_NO;
	Serial.println("Searching for: " + String(id));
	while(--size != 0xff)
		if(id == autoProgs[size].id)
			return size;
	Serial.println("Program not found");
	return 0xff;
}
uint8_t selectedProgram;

uint8_t speed, cycle;
uint8_t autoRed, autoGreen, autoBlue, autoStrobo;

#define FIRST_MENU_SIZE 3
#define FM_PROGRAM 0
#define FM_SPEED 1
#define FM_CYCLE 2
const String firstMenuTexts[FIRST_MENU_SIZE] = {
		"Program", "Speed", "Delay cycle"
	};
uint8_t firstMenuIndex;
void checkIndex(bool increment){
	if(selectedProgram == AP_PROGRAM_OFF)
		firstMenuIndex = FM_PROGRAM;
	else if(selectedProgram != AP_PROGRAM_AUTO && firstMenuIndex == FM_CYCLE)
		if(increment)
			firstMenuIndex = FM_PROGRAM;
		else
			firstMenuIndex = FM_SPEED;
}
void autoModeFirstMenu(){
	if(modeUpdated){
		readAutoSpeedValues(&speed, &cycle);
		selectedProgram = getProgramIndex(getAutoProgram());
		Serial.println(String(selectedProgram));
		checkIndex(true);
		printAMFMsecondLine();
		modeUpdated = false;
	}
	autoProgs[selectedProgram].exec();
	redOut = autoRed;
	greenOut = autoGreen;
	blueOut = autoBlue;
	/*if(blueOut == 255){
		Serial.println("DIOCANENEEEEE");
		delay(10000);
	}*/
	setStrobe(autoStrobo);
}
void printAMFMsecondLine(){
	lcd.setCursor(0, 1);
	lcd.print(firstMenuTexts[firstMenuIndex]  + ":         ");
	uint8_t pointer = 15, value;
	String printS;
	if(firstMenuIndex == FM_PROGRAM){
		printS = autoProgs[selectedProgram].name;
		pointer -= printS.length() - 1;
	} else {
		if(firstMenuIndex == FM_SPEED)
			value = speed;
		else
			value = cycle;
		if(value > 9) pointer--;
		if(value > 99) pointer--;
		printS = String(value + 1);	
	}
	lcd.setCursor(pointer, 1);
	lcd.print(printS);
	lcd.setCursor(15, 1);
}
void amfmConfirmValue(){
	saveAutoSpeedValues(speed, cycle);
}
void amfmChangeValue(uint8_t *value){
	initSelectValueMenu(value, printAMFMsecondLine, 255, 0, amfmConfirmValue, NULL, autoModeFirstMenu); //TODO
}
void amfmEnterPressed(){
	switch(firstMenuIndex){
		case FM_PROGRAM: {
			runMode(mode_autoSelect);
			break;
		}
		case FM_SPEED: {
			amfmChangeValue(&speed);
			break;
		}
		case FM_CYCLE: {
			amfmChangeValue(&cycle);
			break;
		}
	}
}
void amfmLeftPressed(){
	if(--firstMenuIndex == 255)
		firstMenuIndex = FM_CYCLE;
	checkIndex(false);
	printAMFMsecondLine();
}
void amfmRightPressed(){
	if(++firstMenuIndex == FIRST_MENU_SIZE)
		firstMenuIndex = 0;
	checkIndex(true);
	printAMFMsecondLine();
}

uint8_t amsmIndex;
void autoModeSelectMenu(){
	if(modeUpdated){
		amsmIndex = selectedProgram;
		printAMSMsecondLine();
		modeUpdated = false;
	}
	autoModeFirstMenu();
}
String amsmTxt;
void printAMSMsecondLine(){
	amsmTxt = centerText(autoProgs[amsmIndex].name);
	amsmTxt[0] = '<';
	amsmTxt[15] = '>';

	if(amsmIndex == selectedProgram){
		uint8_t i;
		for(i = 1; i < 16; i++){
			//Serial.println(String(i) + " " + mmenu2Txt[i]);
			if(amsmTxt[i] != ' '){
				amsmTxt[i - 1] = 126;
				break;
			}
		}
		for(i = 14; i != 0xff; i--)
			if(amsmTxt[i] != ' '){
				amsmTxt[i + 1] = 127;
				break;
			}
	}
	lcd.setCursor(0, 1);
	lcd.print(amsmTxt);
}
void amsmEnterPressed(){
	selectedProgram = amsmIndex;
	changedProg = true;
	setAutoProgram(autoProgs[selectedProgram].id);
	doneMode();
}
void amsmLeftPressed(){
	if(--amsmIndex == 255)
		amsmIndex = AUTOPROGRAM_NO - 1;
	printAMSMsecondLine();
}
void amsmRightPressed(){
	if(++amsmIndex == AUTOPROGRAM_NO)
		amsmIndex = 0;
	printAMSMsecondLine();
}

struct RGBcolor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};
const RGBcolor RED = {255, 0, 0};
const RGBcolor GREEN = {0, 255, 0};
const RGBcolor BLUE = {0, 0, 255};
const RGBcolor YELLOW = {255, 255, 0};
const RGBcolor CYAN = {0, 255, 255};
const RGBcolor MAGENTA = {255, 0, 255};
const RGBcolor WHITE = {255, 255, 255};
const RGBcolor BLACk = {0, 0, 0};
#define RGB_COLORS_SIZE 3
const RGBcolor rgbColors[RGB_COLORS_SIZE] = {RED, GREEN, BLUE};
#define ALL_COLORS_SIZE 7
const RGBcolor allColors[ALL_COLORS_SIZE] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE};

void setColor(const RGBcolor color);
void setColor(const RGBcolor color){
	autoRed = color.red;
	autoGreen = color.green;
	autoBlue = color.blue;
}
uint16_t nextStaticColorChange;
uint8_t programColorSize, indexProgramColor;
RGBcolor *programColors;
#define STATICCOLOR_MINSPEED 5
void init_program_staticColor(const RGBcolor colors[], uint8_t size);
void init_program_staticColor(const RGBcolor colors[], uint8_t size){
	programColors = colors;
	programColorSize = size;
	indexProgramColor = 0;
	nextStaticColorChange = 1;
}
void program_staticColor(){
	if(!--nextStaticColorChange){
		if(++indexProgramColor >= programColorSize)
			indexProgramColor = 0;
		setColor(programColors[indexProgramColor]);
		nextStaticColorChange = 255 + STATICCOLOR_MINSPEED - speed;
	}
}
int16_t ftcSpeed;
float ftcChangeR, ftcChangeG, ftcChangeB;
float ftcStatusR, ftcStatusG, ftcStatusB;
RGBcolor ftcFinal;
void initFadeToColor(const RGBcolor start, const RGBcolor final, int16_t sp);
void initFadeToColor(const RGBcolor start, const RGBcolor final, int16_t sp){
	ftcStatusR = start.red;
	ftcStatusG = start.green;
	ftcStatusB = start.blue;
	ftcFinal = final;
	ftcSpeed = sp;
	if(sp){
		ftcChangeR = (final.red - start.red) / (float) sp;
		ftcChangeG = (final.green - start.green) / (float) sp;
		ftcChangeB = (final.blue - start.blue) / (float) sp;
	}
	/*Serial.println("S: " + String(start.red) + " " + String(start.green) + " " + String(start.blue));
	Serial.println("F: " + String(final.red) + " " + String(final.green) + " " + String(final.blue));
	Serial.println("C: " + String(ftcChangeR) + " " + String(ftcChangeG) + " " + String(ftcChangeB));
	Serial.println("Speed: " + String(ftcSpeed));*/
}
bool fadeToColorCycle(){
	//Serial.println(String(ftcSpeed));
	if(--ftcSpeed < 0){
		//Serial.println("E: " + String(ftcFinal.red) + " " + String(ftcFinal.green) + " " + String(ftcFinal.blue));
		autoRed = ftcFinal.red;
		autoGreen = ftcFinal.green;
		autoBlue = ftcFinal.blue;
		return true;
	}
	ftcStatusR += ftcChangeR;
	ftcStatusG += ftcChangeG;
	ftcStatusB += ftcChangeB;

	autoRed = ftcStatusR;
	autoGreen = ftcStatusG;
	autoBlue = ftcStatusB;
	
	//if(ftcSpeed <= 1)
	//	Serial.println("Z: " + String(autoRed) + " " + String(autoGreen) + " " + String(autoBlue));
	return false;
}
#define FADECOLOR_MINSPEED 25
void init_program_fadeColor(const RGBcolor colors[], uint8_t size);
void init_program_fadeColor(const RGBcolor colors[], uint8_t size){
	programColors = colors;
	programColorSize = size;
	indexProgramColor = 0;
	initFadeToColor(
			programColors[programColorSize - 1],
			programColors[0],
			255 + FADECOLOR_MINSPEED - speed
		);
}
void program_fadeColor(){
	if(fadeToColorCycle()){
		if(++indexProgramColor >= programColorSize)
			indexProgramColor = 0;
		//Serial.println("pre");
		//delay(1000);
		initFadeToColor(
			programColors[indexProgramColor ? (indexProgramColor - 1) : (programColorSize - 1)],
			programColors[indexProgramColor],
			255 + FADECOLOR_MINSPEED - speed
		);
	}
}

void program_off(){
	if(changedProg){
		autoRed = 0;
		autoGreen = 0;
		autoBlue = 0;
		autoStrobo = 0;
		changedProg = false;
	}
}
void program_rgb(){
	if(changedProg){
		autoStrobo = 0;
		init_program_staticColor(rgbColors, RGB_COLORS_SIZE);
		changedProg = false;
	}
	program_staticColor();
}
void program_rygcbm(){
	if(changedProg){
		autoStrobo = 0;
		init_program_staticColor(allColors, ALL_COLORS_SIZE);
		changedProg = false;
	}
	program_staticColor();
}
void program_fade_rgb(){
	if(changedProg){
		autoStrobo = 0;
		init_program_fadeColor(rgbColors, RGB_COLORS_SIZE);
		changedProg = false;
	}
	program_fadeColor();
}
void program_fade_rygcbm(){
	if(changedProg){
		autoStrobo = 0;
		init_program_fadeColor(allColors, ALL_COLORS_SIZE);
		changedProg = false;
	}
	program_fadeColor();
}
void program_strobo(){
	if(changedProg){
		autoRed = 255;
		autoGreen = 255;
		autoBlue = 255;
		changedProg = false;
	}
	autoStrobo = speed;
}

#define AUTOMODE_MINSPEED 10
uint8_t autoIndexProgram = 255;
int16_t nextProgramDelay;
void program_auto(){
	if(--nextProgramDelay < 0){
		//Serial.println("CHANGING");
		autoIndexProgram++;
		if(autoIndexProgram == AP_PROGRAM_AUTO)
			autoIndexProgram++;
		if(autoIndexProgram == AP_PROGRAM_OFF)
			autoIndexProgram++;
		if(autoIndexProgram >= AUTOPROGRAM_NO)
			autoIndexProgram = 0;
		changedProg = true;
		nextProgramDelay = (255 + AUTOMODE_MINSPEED - cycle) << 3;
	}
	//Serial.print(String(autoIndexProgram) + " " + String(nextProgramDelay));
	autoProgs[autoIndexProgram].exec();
	//Serial.println("\tDone");
}