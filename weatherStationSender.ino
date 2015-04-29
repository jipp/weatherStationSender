#include <Streaming.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <LowPower.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

//#define DEBUG

#define ONE_WIRE_BUS 4
#define DHTPIN 3
#define CE_PIN 9
#define CSN_PIN 10
#define PRINTF_BUF 20
#define DHTTYPE DHT22

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
uint8_t getDeviceCount = 0;

DHT dht(DHTPIN, DHTTYPE);

RF24 radio(CE_PIN, CSN_PIN);
const uint64_t pipe = 0xF0F0F0F0E1LL;

Adafruit_BMP085 bmp;
bool BMP085 = false;

char buf[PRINTF_BUF];
char str[8];

void setup() {
#ifdef DEBUG
  long start = millis();
#endif
  Serial.begin(115200);
  setupRF24();
  setupDS18x20();
  setupDHT22();
  setupBMP085();
#ifdef DEBUG
  Serial << "setup done: " << millis() - start << " msec" << endl << endl;
#endif
}

void loop() {
#ifdef DEBUG
  long start = millis();
#endif
  getDS18x20();
  getDHT22();
  getBMP085();
  sendData("eof");
  sleep();
#ifdef DEBUG
  Serial << "loop done: " << millis() - start << " msec" << endl;
#endif
}

void sleep() {
  radio.powerDown();
  for (int i = 0; i < 10; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  radio.powerUp();
}

void setupRF24() {
  printf_begin();
  radio.begin();
  radio.setAutoAck(1);
  radio.setRetries(15, 15);
  radio.enableDynamicPayloads();
  radio.openWritingPipe(pipe);
#ifdef DEBUG
  radio.printDetails();
#endif
}

void setupDS18x20() {
  sensors.begin();
  getDeviceCount = sensors.getDeviceCount();
  if (getDeviceCount != 0) {
#ifdef DEBUG
    Serial << getDeviceCount << "x DS18x20" << endl;
#endif
    sensors.setResolution(TEMP_12_BIT);
  }
}

void getDS18x20() {
  if (getDeviceCount > 0) {
    sensors.requestTemperatures();
    sprintf(buf, "%dx DS18x20", getDeviceCount);
    sendData(buf);
    for (int i = 0; i < getDeviceCount; i++) {
      dtostrf(sensors.getTempCByIndex(i), 3, 2, str);
      sprintf(buf, " t(%d)=%s", i, str);
      sendData(buf);
    }
  }
}

void setupDHT22() {
#ifdef DEBUG
  dht.begin
  Serial << "DHT22" << endl;
#endif
}

void getDHT22() {
  sendData("DHT22");
  dtostrf(dht.readTemperature(), 3, 2, str);
  sprintf(buf, " t=%s", str);
  sendData(buf);
  dtostrf(dht.readHumidity(), 3, 2, str);
  sprintf(buf, " f=%s", str);
  sendData(buf);
}

void setupBMP085() {
  if (bmp.begin()) {
    BMP085 = true;
#ifdef DEBUG
    Serial << "BMP085" << endl;
#endif
  }
}

void getBMP085() {
  if (BMP085) {
    sendData("BMP085");
    dtostrf(bmp.readTemperature(), 3, 2, str);
    sprintf(buf, " t=%s", str);
    sendData(buf);
    dtostrf(bmp.readPressure() / 100, 3, 2, str);
    sprintf(buf, " p=%s", str);
    sendData(buf);
    dtostrf(bmp.readAltitude(), 3, 2, str);
    sprintf(buf, " h=%s", str);
    sendData(buf);
    dtostrf(bmp.readSealevelPressure() / 100, 3, 2, str);
    sprintf(buf, " p0=%s", str);
    sendData(buf);
    dtostrf(bmp.readAltitude(101500), 3, 2, str);
    sprintf(buf, " hr=%s", str);
    sendData(buf);
  }
}

void sendData(char *buf) {
  Serial << buf << endl;
#ifdef DEBUG
  if (!radio.write(buf, strlen(buf))) {
    Serial << "transmission failed" << endl;
  }
#else
  radio.write(buf, strlen(buf));
#endif
  delay(100);
}


