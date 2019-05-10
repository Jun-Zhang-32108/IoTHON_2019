/**
    Required libraries:
      - MKRNB
      - PubSubClient
      - Adafruit_Sensor
      - Adafruit_BME680
     All can be added in the Anduino library
**/

//Author: Yichen Cao, Jun Zhang

/**************************************************/
//Path of brokers
#define MQTT_TOPIC_TEMPERATURE "iothon/myteam/temperature"
#define MQTT_TOPIC_RSSI        "iothon/myteam/rssi"
#define MQTT_TOPIC_STATE       "iothon/myteam/status"
#define MQTT_TOPIC_COUNTER     "iothon/myteam/counter"
#define MQTT_TOPIC_HUMIDITY    "iothon/myteam/humidity"
#define MQTT_TOPIC_PRESSURE    "iothon/myteam/pressure"
#define MQTT_TOPIC_GASRESISTANCE    "iothon/myteam/gasResistance"

#define MY_SERVER  "18.220.164.248" //Server IP
/****************************************************/

/*********************Sensor*********************/
#include <Wire.h>                             
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#define BME_SCK 9
#define BME_MISO 10
#define BME_MOSI 8
#define BME_CS 7
#define SEALEVELPRESSURE_HPA (1013.25)

//Adafruit_BME680 bme; // I2C
Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);
/************************************************/

#include <MKRNB.h>
#include <PubSubClient.h>


const char PIN_NUMBER[] = "0000"; //PIN_NUMBER[] = "" if there is no pin
const char APN[] = ""; // 

#define MQTT_PUBLISH_DELAY        1000 // 1 seconds
#define MQTT_CLIENT_ID           "mkrnb1500iothon"

const char *MQTT_SERVER   = MY_SERVER;
const char *MQTT_USER     = "mqttuser";     // NULL for no authentication
const char *MQTT_PASSWORD = "mqttpassword"; // NULL for no authentication

int counter=0;

NB           nbAccess(true); // NB access: use a 'true' parameter to enable debugging
GPRS         gprsAccess;     // GPRS access
NBClient     tcpSocket;
PubSubClient mqttClient(tcpSocket);

void setup() {
  Serial.begin(115200);
  while (! Serial)
    ;
    
  initBME680();
  initMQTT();
}

long lastMsgTime = 0;

void loop() {
  runMQTT();
}

float readRSSI() {
  String rssi;

  MODEM.send("AT+CESQ");
  MODEM.waitForResponse(100, &rssi);
  Serial.print("RSSI=");
  Serial.println(rssi);

  if (rssi.startsWith("+CESQ: ")) {
    rssi.remove(0, 7);
    return rssi.toFloat();
  }
  return NAN;
}

void mqttConnectIfNeeded() {
  while (!mqttClient.connected()) {
    Serial.println("Connect: starting...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("Connect: connected.");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("Connect: failed, state=");
      Serial.print(mqttClient.state());
      Serial.println(". Trying again in 5 seconds.");
      delay(5000);
    }
  }
}

void mqttPublish(String topic, float payload) {
  Serial.print("Publish: ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic.c_str(), String(payload).c_str(), true);
}
void initBME680(){
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}
void readBME680(){
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
}
void initMQTT(){
  Serial.println("MKR NB 1500 MQTT client starting.");

  Serial.println("Connect: NB-IoT / LTE Cat M1 network (may take several minutes)...");
  while (nbAccess.begin(PIN_NUMBER, APN) != NB_READY) {
    Serial.println("Connect: NB-IoT: failed.  Retrying in 10 seconds.");
    delay(10000);
  }
  Serial.println("Connect: NB-IoT: connected.");

  Serial.println("Acquire: PDP context...");
  while (gprsAccess.attachGPRS() != GPRS_READY) {
    Serial.println("Acquire: PDP context: failed.  Retrying in 10 seconds.");
    delay(10000);
  }
  Serial.println("Acquire: PDP context: acquired.");
#if 1 /* Using newer version of the library */
  Serial.print("Acquire: PDP Context: ");
  Serial.println(nbAccess.readPDPparameters());
#endif

  mqttClient.setServer(MQTT_SERVER, 1883);
}

void runMQTT(){
  Serial.println("Looping: start...");
  if (!mqttClient.loop()) {
    mqttConnectIfNeeded();
  }

  long now = millis();
  if (now - lastMsgTime > MQTT_PUBLISH_DELAY) {
    lastMsgTime = now;

    if (! bme.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
    //temperature
    float temperature = bme.temperature;
    mqttPublish(MQTT_TOPIC_TEMPERATURE, temperature);

    //humidity
    float humidity = bme.humidity;
    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity);

    //pressure
    float pressure = bme.pressure;
    mqttPublish(MQTT_TOPIC_PRESSURE, pressure);

    // voc gas resistance
    float gasResistance = bme.gas_resistance;
    mqttPublish(MQTT_TOPIC_GASRESISTANCE, gasResistance);

    //RSSI
    float rssi = readRSSI();
    mqttPublish(MQTT_TOPIC_RSSI, rssi);

    //counter of frame
    mqttPublish(MQTT_TOPIC_COUNTER, counter++);
  }
  Serial.println("Looping: delay...");
  delay(1000);
  Serial.println("Looping: done.");
}
