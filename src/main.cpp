/*!
  * @file  initiativereport.ino
  * @brief The sensor actively reports all data
  * @n Experimental method: Connect the sensor communication pin to the main control, then burn codes into it. 
  * @n Communication mode selection, dial switch SEL:0: IIC, 1: UART
@n I2C address selection, the default I2C address is 0x74, A1 and A0 are combined into 4 types of IIC addresses
                | A1 | A0 |
                | 0  | 0  |    0x74
                | 0  | 1  |    0x75
                | 1  | 0  |    0x76
                | 1  | 1  |    0x77   default i2c address
  * @n Experimental phenomenon: Print all data via serial port
*/

// DFRobot Gravity H2S Sensor (SEN0467)
// https://www.dfrobot.com/product-2511.html

#include "M5Unified.h"
#include "DFRobot_MultiGasSensor.h"
#include <SD.h>

//Enabled by default, use IIC communication at this time. Use UART communication when disabled

#define I2C_ADDRESS    0x74
DFRobot_GAS_I2C gas(&Wire ,I2C_ADDRESS);

File logFile;
uint32_t t0 = 0;

#define LOG_FILENAME "/log.csv"
bool fLogging = false;

void ShowStatus(){
	M5.Display.fillRect(0, 20, 120, 40, BLACK);
	M5.Display.setCursor(0, 20);
	if (fLogging == true){
		M5.Lcd.setTextColor(GREEN);
		M5.Display.printf("Logging");
	}
	else{
		M5.Lcd.setTextColor(RED);
		M5.Display.printf("Stopped");
	}
}

void setup() {

  M5.begin();
	M5.Display.setTextScroll(true);
	M5.Display.setTextSize(2);

  while(!gas.begin())
  {
		M5.Display.setCursor(0, 0);
    M5.Display.printf("No Sensor\n");
    delay(1000);
  }
  printf("The device is connected successfully!\n");
	SPI.begin();
  while(false == 	SD.begin(GPIO_NUM_4, SPI, 15000000)){
		M5.Display.setCursor(0, 0);
		M5.Display.println("no SD card");
		delay(500);
	}
	M5.Display.clear();

	ShowStatus();

  gas.changeAcquireMode(gas.PASSIVITY);
  delay(1000);

  gas.setTempCompensation(gas.OFF);
}

uint16_t px = 0;
#define X 320
uint16_t val[X][1]; // 1 value: H2S
uint16_t color[] = {PURPLE};

uint16_t conv_value(float value, float min, float max) {
	int16_t v = (value - min) / (max - min) * 240;
	if (v < min) v = min;
	else if (v > max) v = max;
	return(v);
}	

#define MAX_conH2S 100 // maximum value: to be fixed

void loop() {
//  printf("Ambient:%s / Cons:%f[%vol] / Temp:%f[degC]\n", gas.queryGasType().c_str(), gas.readGasConcentrationPPM(), gas.readTempC());
//  M5.Display.printf("%.2f[%%vol] %.2f[degC]\n", gas.readGasConcentrationPPM(), gas.readTempC());
  uint16_t error;

	float conH2S, temp;

	conH2S = gas.readGasConcentrationPPM();
	temp = gas.readTempC();

	printf("%.2f,%.2f\n", conH2S, temp);
	val[px][0] = conv_value(conH2S, 0, MAX_conH2S);
	px = (px + 1) % X;
	uint16_t x, p;
	p = px;
	for (x = 0; x < X; x++){
		uint8_t t = 0;
		M5.Display.drawFastVLine(x, 0, 240, BLACK);
		M5.Display.drawPixel(x, 120, LIGHTGREY);
		if (x % 4 == 0){
			M5.Display.drawPixel(x, 60, LIGHTGREY);
			M5.Display.drawPixel(x, 180, LIGHTGREY);
		}
		for (t = 4; t < 8; t++) {
			if (t < 4 || (t >= 4 && !isnan(val[px][t]))){
				M5.Display.drawPixel(x, 240 - val[p][t], color[t]);
			}
		}
		p = (p + 1) % X;
	}
	M5.Lcd.setCursor(0,0);
	M5.Lcd.setTextColor(color[0]); M5.Lcd.printf("H2S(0-%d):%.2f/%.1f[degC]\n", MAX_conH2S, conH2S, temp);
	ShowStatus();

	uint32_t tm = millis() - t0;
	logFile.printf("%d,%f,%f\n", tm, conH2S, temp);

	for (uint8_t i = 0; i < 100; i++){
		M5.update();
		if (M5.BtnA.wasClicked()){
			if (fLogging == false){
				t0 = millis();
				fLogging = true;
				logFile = SD.open(LOG_FILENAME, "a");
				logFile.printf("time[ms],H2S,temp\n");
				i = 100;
				ShowStatus();
			}
			else{
				logFile.close();
				fLogging = false;
				i = 100;
				ShowStatus();
			}
		}
    delay(10);
	}
  delay(1000);
}
