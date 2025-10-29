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
	M5.Display.fillRect(200, 0, 120, 20, BLACK);
	M5.Display.setCursor(200, 0);
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
    printf("NO Deivces !\n");
    delay(1000);
  }
  printf("The device is connected successfully!\n");
/*
	SPI.begin();
  	while(false == 	SD.begin(GPIO_NUM_4, SPI, 15000000)){
		M5.Display.setCursor(200, 0);
		M5.Display.println("no SD");
		delay(500);
	}
	ShowStatus();
*/

  gas.changeAcquireMode(gas.PASSIVITY);
  delay(1000);

  gas.setTempCompensation(gas.OFF);
}

void loop() {
  printf("Ambient:%s / Cons:%f[%vol] / Temp:%f[degC]\n", gas.queryGasType().c_str(), gas.readGasConcentrationPPM(), gas.readTempC());
  M5.Display.printf("%.2f[%%vol] %.2f[degC]\n", gas.readGasConcentrationPPM(), gas.readTempC());
  delay(1000);
}

/*
uint16_t px = 0;
#define X 320
uint16_t val[X][8]; // 8 values: PM1.0, PM2.5, PM4.0, PM10.0, RH, T, VOC, NOx
uint16_t color[] = {RED, PURPLE, MAGENTA, ORANGE, CYAN, YELLOW, GREEN, SKYBLUE};

uint16_t conv_value(float value, float min, float max) {
	int16_t v = (value - min) / (max - min) * 240;
	if (v < min) v = min;
	else if (v > max) v = max;
	return(v);
}	

void loop() {
  uint16_t error;

	float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex;

	error = sen5x.readMeasuredValues(
		massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0,
		ambientHumidity, ambientTemperature, vocIndex, noxIndex);

	// value range:
	// massConcentration: 0-1000 [ug/m3]
	// VOX, NOX: 1-500 (10-30sec) [index]

	if (error) printSen55ErrorMessage("Error trying to execute readMeasuredValues():", error);
	else {
		printf("%f,%f,%f,%f,", massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0);
	  if (isnan(ambientHumidity)) printf("n/a,"); else printf("%f, ", ambientHumidity);
	  if (isnan(ambientTemperature)) printf("n/a,"); else printf("%f, ", ambientTemperature);
	  if (isnan(vocIndex)) printf("n/a,"); else printf("%f, ", vocIndex);
	  if (isnan(noxIndex)) printf("n/a\n"); else printf("%f\n", noxIndex);
		val[px][0] = conv_value(massConcentrationPm1p0, 0, 1000);
		val[px][1] = conv_value(massConcentrationPm2p5, 0, 1000);
		val[px][2] = conv_value(massConcentrationPm4p0, 0, 1000);
		val[px][3] = conv_value(massConcentrationPm10p0, 0, 1000);
		val[px][4] = conv_value(ambientHumidity, 0, 100);
		val[px][5] = conv_value(ambientTemperature, 0, 100);
		val[px][6] = conv_value(vocIndex, 1, 500);
		val[px][6] = conv_value(noxIndex, 1, 500);
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
		M5.Lcd.setTextColor(color[0]); M5.Lcd.printf("PM10(0-1000)\n");
		M5.Lcd.setTextColor(color[1]); M5.Lcd.printf("PM2.5(0-1000)\n");
		M5.Lcd.setTextColor(color[2]); M5.Lcd.printf("PM40(0-1000)\n");
		M5.Lcd.setTextColor(color[3]); M5.Lcd.printf("PM10(0-1000)\n");
		M5.Lcd.setTextColor(color[4]); M5.Lcd.printf("Hum(0-100)\n");
		M5.Lcd.setTextColor(color[5]); M5.Lcd.printf("Temp(0-100)\n");
		M5.Lcd.setTextColor(color[6]); M5.Lcd.printf("VOX(1-500)\n");
		M5.Lcd.setTextColor(color[7]); M5.Lcd.printf("NOx(1-500)\n");
		ShowStatus();
		uint32_t tm = millis() - t0;
		logFile.printf("%d,%f,%f,%f,%f,%f,%f,%f,%f\n", tm, massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
			massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex);
	}
	for (uint8_t i = 0; i < 100; i++){
		M5.update();
		if (M5.BtnA.wasClicked()){
			if (fLogging == false){
				t0 = millis();
				fLogging = true;
				logFile = SD.open(LOG_FILENAME, "a");
				logFile.printf("time[ms],PM10,PM2.5,PM40,PM10,Hum,Temp,VOX,NOx\n");
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
}
  */