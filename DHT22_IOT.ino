/*
    Name:       DHT22_IOT.ino
    Created:	2019/1/31 11:58:46
    Author:     LAPTOP-MF5E2R69\lx200
*/
//#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_ALIGENIE_SENSOR

#define DHTPIN D4
#define DHTTYPE DHT22

#include <Blinker.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Arduino.h>
#include <U8g2lib.h>
//#include <ArduinoJson.h>

//网络信息
char auth[] = "d5d0276fc9c5";
char ssid[] = "Xiaomi_102";
char pswd[] = "ljlljlljl";

//定义数据，传感器
BlinkerNumber HUMI("humi");
BlinkerNumber TEMP("temp");
//BlinkerNumber PRES("pres");
DHT dht(DHTPIN, DHTTYPE);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 13, /* reset=*/ U8X8_PIN_NONE);
//Adafruit_BMP085 bmp;
String weather_hangzhou;
String wea_tmp, wea_hum;
uint32_t read_time = 0, wea_time = 0;
float humi_read, temp_read;
//StaticJsonBuffer<200> jsonBuffer;


void dataRead(const String & data){
	BLINKER_LOG("Blinker readString: ", data);

	Blinker.vibrate();

	uint32_t BlinkerTime = millis();
	Blinker.print(BlinkerTime);
	Blinker.print("millis", BlinkerTime);
}

//心跳包
void heartbeat(){
	HUMI.print(humi_read);
	TEMP.print(temp_read);
	//PRES.print(p_read);
}

//天猫精灵查询接口
void aligenieQuery(int32_t queryCode){
	BLINKER_LOG("AliGenie Query codes: ", queryCode);

	switch (queryCode){
		case BLINKER_CMD_QUERY_ALL_NUMBER: {
			BLINKER_LOG("AliGenie Query All");
			BlinkerAliGenie.temp(temp_read);
			BlinkerAliGenie.humi(humi_read);
			BlinkerAliGenie.print();
			break;
		}
		case BLINKER_CMD_QUERY_TEMP_NUMBER: {
			BLINKER_LOG("AliGenie Query Temp");
			BlinkerAliGenie.temp(temp_read);
			BlinkerAliGenie.print();
			break;
		}
		case BLINKER_CMD_QUERY_HUMI_NUMBER: {
			BLINKER_LOG("AliGenie Query Humi");
			BlinkerAliGenie.humi(humi_read);
			BlinkerAliGenie.print();
			break;
		}
		default:
			break;
	}
}

void setup(){
	Serial.begin(115200);
	BLINKER_DEBUG.stream(Serial);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	pinMode(D5, OUTPUT);
	digitalWrite(D5, HIGH);

	Blinker.begin(auth, ssid, pswd);
	Blinker.attachData(dataRead);
	Blinker.attachHeartbeat(heartbeat);
	BlinkerAliGenie.attachQuery(aligenieQuery);

	//bmp.begin();
	dht.begin();
	u8g2.begin();
	u8g2.setFont(u8g2_font_bitcasual_t_all);
}

void loop(){
	Blinker.run();
	//每隔2秒读传感器
	if (read_time == 0 || (millis() - read_time) >= 5000){
		read_time = millis();

		//读取传感器
		float h = dht.readHumidity();
		float t = dht.readTemperature();
		//float p = bmp.readPressure();

		if (isnan(h) || isnan(t)) {
			BLINKER_LOG("Failed to read from DHT sensor!");
			return;
		}
		/*if (isnan(p)) {
			BLINKER_LOG("Failed to read from BMP180 sensor!");
			return;
		}*/

		float hic = dht.computeHeatIndex(t, h, false);
		humi_read = h;
		temp_read = t;
		//p_read = p;

		BLINKER_LOG("Humidity: ", h, " %");
		BLINKER_LOG("Temperature: ", t, " *C");
		BLINKER_LOG("Heat index: ", hic, " *C");
		//u8g2.clearDisplay();          // 清屏
		u8g2.clearBuffer();    // 清空显示设备内部缓冲区

		u8g2.drawStr(0, 8, "My weather station");
		u8g2.drawStr(0, 18, "Temp.: ");
		u8g2.setCursor(64, 18);
		u8g2.print(temp_read);
		u8g2.print(" C");

		u8g2.drawStr(0, 28, "Humi.: ");
		u8g2.setCursor(64, 28);
		u8g2.print(humi_read);
		u8g2.print("%");
		
		u8g2.drawStr(0, 48, "Powered by lx2005");
		/*
		u8g2.drawStr(0, 38, "Today's weather");
		u8g2.drawStr(0, 48, "Temp.: ");
		u8g2.setCursor(64, 48);
		u8g2.print(wea_tmp);
		u8g2.print(" C");

		u8g2.drawStr(0, 58, "Humi.: ");
		u8g2.setCursor(64, 58);
		u8g2.print(wea_hum);
		//u8g2.print("%");
		*/

		u8g2.sendBuffer();
		//BLINKER_LOG("Pressure: ", p, "Pa");
		BLINKER_LOG("Weather: ", weather_hangzhou);
	}

	//每分钟刷新天气
	
	/*
	if (wea_time == 0 || (millis() - wea_time) >= 10000) {
		wea_time = millis();
		String postRequest = (String)"GET /api/weather/city/101030100 HTTP/1.1\r\nHost: t.weather.sojson.com\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: zh-CN,zh;q=0.9\r\n";
		client.print(postRequest);
		BLINKER_LOG("Request: ", postRequest);
		weather_hangzhou = client.readStringUntil('\n');
		BLINKER_LOG("Weather: ", weather_hangzhou);
		JsonObject& root = jsonBuffer.parseObject(weather_hangzhou);
		wea_tmp = root["data"]["wendu"].as<String>();
		wea_hum = root["data"]["shidu"].as<String>();
	}
	*/

}
