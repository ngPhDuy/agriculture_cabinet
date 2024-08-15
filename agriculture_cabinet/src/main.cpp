#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include "RelayStatus.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include "DHT20.h"

#define IO_USERNAME "DuyNgPh"
#define IO_KEY "___"

#define WIFI_SSID "ACLAB"
#define WIFI_PASS "ACLAB2023"

#define LED_PIN 2
#define TXD 8
#define RXD 9
#define BAUD_RATE 9600

#define COLLECTION_INTERVAL 10000

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *status = io.feed("status");
AdafruitIO_Feed *sensor_data = io.feed("sensor_data");

DHT20 dht;

unsigned long lastMillis = 0;

void sendModbusCommand(const uint8_t command[], size_t length)
{
  for (size_t i = 0; i < length; i++)
  {
    Serial2.write(command[i]);
  }
}

void handleMessage(AdafruitIO_Data *data)
{
  String message = data->value();

  if (message.startsWith("!RELAY") && message.endsWith("#"))
  {
    int indexStart = message.indexOf('!') + 6;
    int indexEnd = message.indexOf(':');
    String indexStr = message.substring(indexStart, indexEnd);
    int index = indexStr.toInt();

    int statusStart = indexEnd + 1;
    int statusEnd = message.indexOf('#');
    String statusStr = message.substring(statusStart, statusEnd);

    // Debug prints
    Serial.print("Raw message: ");
    Serial.println(message);
    Serial.print("Index string: ");
    Serial.println(indexStr);
    Serial.print("Index: ");
    Serial.println(index);
    Serial.print("Status string: ");
    Serial.println(statusStr);

    // Send the Modbus command for the specific relay
    if (statusStr == "ON" && index < (sizeof(relay_ON) / sizeof(relay_ON[0])))
    {
      sendModbusCommand(relay_ON[index], sizeof(relay_ON[0]));
      Serial.println("Relay " + String(index) + " turned ON");
    }
    else if (statusStr == "OFF" && index < (sizeof(relay_OFF) / sizeof(relay_OFF[0])))
    {
      sendModbusCommand(relay_OFF[index], sizeof(relay_OFF[0]));
      Serial.println("Relay " + String(index) + " turned OFF");
    }
    else
    {
      Serial.println("Invalid command");
    }

    String sendData = String(index) + '-' + statusStr;
    status->save(sendData);
    Serial.println("Data sent to Adafruit IO: " + sendData);
  }
}

void sendSensorData(void* params)
{
  while (true)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis >= COLLECTION_INTERVAL)
    {
      lastMillis = currentMillis;
      Serial.println("send data");
      dht.read();
      // Read sensor data
      float temperature = dht.getTemperature();
      float humidity = dht.getHumidity();
      String data = String(temperature) + "_" + String(humidity);
      Serial.println(data);

      // Send sensor data to Adafruit IO
      sensor_data->save(data);
    }
  }
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(48, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, TXD, RXD);

  sendModbusCommand(relay_OFF[0], sizeof(relay_OFF[0]));

  while (!Serial)
    ;

  Serial.println("Connecting to Adafruit IO");
  io.connect();
  status->onMessage(handleMessage);

  while (io.status() < AIO_CONNECTED)
  {
    Serial.println("Can't connect to Adafruit IO");
    digitalWrite(48, LOW);
    delay(1000);
    digitalWrite(48, HIGH);
  }

  Serial.println();
  Serial.println(io.statusText());
  status->get();
  digitalWrite(48, HIGH);

  Wire.begin();
  dht.begin();
  xTaskCreate(sendSensorData, "sendSensorData", 4096, NULL, 0, NULL);
}

void loop()
{
  io.run();
}