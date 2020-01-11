#include <LiquidCrystal_I2C.h>

struct Color {
	String name;
	uint8_t value;
};

#define RED 0
Color colorRED = {
	"Red",
	0
};
#define GREEN 1
Color colorGREEN = {
	"Green",
	0
};
#define BLUE 2
Color colorBLUE = {
	"Blue",
	0
};
#define STROBO 3
Color colorSTROBE = {
	"Strobe",
	0
};

#define COLORS_SIZE 4
Color colors[COLORS_SIZE] = {colorRED, colorGREEN, colorBLUE, colorSTROBE};
uint8_t selectedColor;
void rgbMode(){
	if(modeUpdated){
		readRGBSvalues(&colors[RED].value, &colors[GREEN].value, &colors[BLUE].value, &colors[STROBO].value);
		updateRGBOutScreen();
		modeUpdated = false;
	}
}

void updateRGBOutScreen(){
	outRGBValues();
	printRGBSelection();
}
void outRGBValues(){
	redOut = colors[RED].value;
	greenOut = colors[GREEN].value;
	blueOut = colors[BLUE].value;
	setStrobe(colors[STROBO].value);
}
void printRGBSelection(){
	lcd.setCursor(0, 1);
	lcd.print("<" + colors[selectedColor].name  + "          ");
	uint8_t pointer = 14;
	if(colors[selectedColor].value > 9) pointer--;
	if(colors[selectedColor].value > 99) pointer--;
	lcd.setCursor(pointer, 1);
	lcd.print(String(colors[selectedColor].value) + ">");
	lcd.setCursor(14, 1);
}

void confirmedRGBSvalue(){
	saveRGBSvalues(colors[RED].value, colors[GREEN].value, colors[BLUE].value, colors[STROBO].value);
}
void rgbEnterPressed(){
	initSelectValueMenu((uint8_t*) &colors[selectedColor].value, updateRGBOutScreen, 255, 0, confirmedRGBSvalue, NULL, NULL);
}
void rgbLeftPressed(){
	if(--selectedColor == 255)
		selectedColor = COLORS_SIZE - 1;
	printRGBSelection();
}
void rgbRightPressed(){
	if(++selectedColor == COLORS_SIZE)
		selectedColor = 0;
	printRGBSelection();
}