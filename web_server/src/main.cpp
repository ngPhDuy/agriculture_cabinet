#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LOOP_INTERVAL 2000
// Replace with your network credentials
const char* ssid = "ACLAB";
const char* password = "ACLAB2023";
const char* mqtt_server = "io.adafruit.com";
const char* io_username = "DuyNgPh";
const char* io_key = "aio_PreS83UdbmAfs8DTh96bfnrXhV84";
const char* io_feed = "DuyNgPh/feeds/status";
const char* last_feed = "DuyNgPh/feeds/sensor_data";

unsigned long lastLoopMillis = millis();

const String DATABASE_IP = "http://172.28.182.102/web_for_aclab";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiClient espClient; 
PubSubClient client(espClient);
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

/////////////////////////////////////////////////////
///WEB SOCKET////
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
//send json data
//Input form is: temp_humi
void sendToSocket(String data) {
  String temp = data.substring(0, data.indexOf("_"));
  String humi = data.substring(data.indexOf("_") + 1);
  JsonDocument doc;
  doc["temperature"] = temp;
  doc["humidity"] = humi;
  String json;
  serializeJson(doc, json);
  ws.textAll(json);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/save_data.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"temperature\":" + temp + ",\"humidity\":" + humi + "}";

  int httpCode = httpClient.POST(payload);

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    Serial.println("Data saved successfully");
  } else {
    Serial.println("Fail to save data");
  }

  httpClient.end();
}

/////////////////////////////////////////////////////
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", io_username, io_key)) {
            Serial.println("connected");
            client.subscribe(last_feed);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void sendMQTTMessage(const char* message) {
  // xSemaphoreTake(xMutex, portMAX_DELAY);
  if (!client.connected()) {
    reconnect();
  }

  client.publish(io_feed, message);
  // xSemaphoreGive(xMutex);
}

void getLastMessage(char* topic, byte* payload, unsigned int length) {
  // xSemaphoreTake(xMutex, portMAX_DELAY);
    payload[length] = '\0';
    String message = String((char*)payload);
    sendToSocket(message);
  // xSemaphoreGive(xMutex);
}
/////////////////////////////////////////////////////
void maintainConenctionTask(void* params) {
  while (1) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastLoopMillis >= LOOP_INTERVAL) {
      Serial.println("Looping");
      // xSemaphoreTake(xMutex, portMAX_DELAY);
      client.loop();
      // xSemaphoreGive(xMutex);
      lastLoopMillis = currentMillis;
      Serial.println("Looped");
    }
  }
}
// Replaces placeholder with button section in your web page

String getDataForIndex(const String& var) {
  if (var == "DEVICES") {
    String rtString = "";
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/get_devices.php");
    int httpCode = httpClient.GET();
    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);

      for (JsonObject object : doc.as<JsonArray>()) {
        rtString += "<div class=\"device\">";
        rtString += "<div class=\"device-info\">";
        rtString +=  "<h3>" + object["name"].as<String>() + "</h3>";
        rtString += "<p class=\"description\">" + object["description"].as<String>() + "</p>";
        rtString += "</div>";
        rtString += "<label class=\"switch\">";
        if (object["state"].as<String>() == "1") {
          rtString += "<input type=\"checkbox\" class=\"toggle\" data-id=\"" + object["id"].as<String>() + "\" checked>";
        } else {
          rtString += "<input type=\"checkbox\" class=\"toggle\" data-id=\"" + object["id"].as<String>() + "\">";
        }
        rtString += "<span class=\"slider round\"></span>";
        rtString += "</label>";
        rtString += "</div>";
      }
    }
    httpClient.end();
    return rtString;
  } else if (var == "ON_DEVICES_PERCENTAGE") {
    String rtString = "";
    int totalDevices = 0;
    int onDevices = 0;
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/get_devices.php");
    int httpCode = httpClient.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);

      totalDevices = doc.size();
      for (JsonObject object : doc.as<JsonArray>()) {
        if (object["state"].as<String>() == "1") {
          onDevices++;
        }
      }

      rtString += "<span class=\"device-count\">";
      rtString += String(onDevices);
      rtString += "/";
      rtString += String(totalDevices);
      rtString += "</span>";
    }
    httpClient.end();
    return rtString;
  }
  return String();
}

String getDataForDevices(const String& var) {
  if (var == "DEVICES") {
    String rtString = "";
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/get_devices.php");
    int httpCode = httpClient.GET();
    int count = 0;

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();
      
      JsonDocument doc;
      deserializeJson(doc, payload);

      for (JsonObject object : doc.as<JsonArray>()) {
        count++;
        rtString += "<tr>";
        rtString += "<td>" + String(count) + "</td>";
        rtString += "<td>" + object["name"].as<String>() + "</td>";
        rtString += "<td>" + object["id"].as<String>() + "</td>";
        rtString += "<td class=\"left\">" + object["description"].as<String>() + "</td>";
        rtString += object["state"].as<String>() == "1" ? "<td class=\"on\">On</td>" : "<td class=\"off\">Off</td>";
        rtString += "<td class=\"buttons-wrapper\">";
        rtString += "<button class=\"edit-button\" data-id=\"" + object["id"].as<String>() + "\" data-name=\"" + object["name"].as<String>() + "\" data-description=\"" + object["description"].as<String>() + "\">Sửa</button>";
        rtString += "<button class=\"delete-button\" data-id=\"" + object["id"].as<String>() + "\">Xóa</button>";
        rtString += "</td>";
        rtString += "</tr>";
      }
    }
    httpClient.end();
    return rtString;
  }
  return String();
}

String getDataForTimers(const String& var) {
  if (var == "TIMERS") {
    String rtString = "";
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/get_timers.php");
    int httpCode = httpClient.GET();
    int count = 0;

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);

      for (JsonObject object : doc.as<JsonArray>()) {
        count++;
        rtString += "<tr>";
        rtString += "<td>" + String(count) + "</td>";
        rtString += "<td>" + object["name"].as<String>() + "</td>";
        rtString += "<td data-id=\"" + object["device_id"].as<String>() + "\">" + object["device_name"].as<String>() + "</td>";
        rtString += "<td>" + object["start_time"].as<String>() + "</td>";
        rtString += "<td>" + object["duration"].as<String>() + "</td>";
        rtString += "<td><label class='switch'>";
        rtString += object["state"].as<String>() == "1" ? "<input type='checkbox' class='toggle' timer-id=\"" + object["id"].as<String>() + "\" checked>" : "<input type='checkbox' class='toggle' timer-id=\"" + object["id"].as<String>() + "\">";
        rtString += "<span class='slider round'></span>";
        rtString += "</label></td>";
        rtString += "<td class=\"buttons-wrapper\">";
        rtString += "<button class=\"edit-button\" timer-id=\"" + object["id"].as<String>() + "\">Sửa</button>";
        rtString += "<button class=\"delete-button\" timer-id=\"" + object["id"].as<String>() + "\">Xóa</button>";
        rtString += "</td>";
        rtString += "</tr>";
      }
    }
    httpClient.end();
    return rtString;
  } else if (var == "DEVICE_OPTIONS") {
    String rtString = "";
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/get_devices.php");
    int httpCode = httpClient.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);

      for (JsonObject object : doc.as<JsonArray>()) {
        rtString += "<option value=\"" + object["id"].as<String>() + "\">" + object["name"].as<String>() + "</option>";
      }
    }
    httpClient.end();
    return rtString;
  }
  return String();
}

void controlDeviceHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String relay_id = doc["relay_id"].as<String>();
  String state = doc["state"].as<String>();

  String mqttMess = "!RELAY" + relay_id + (state == "1" ? ":ON#" : ":OFF#");

  Serial.println(relay_id + ", " + state);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/control_device.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"relay_id\":" + relay_id + ",\"state\":" + state + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Device updated successfully";
    response["status"] = 200;
    serializeJson(response, responseString);

    sendMQTTMessage(mqttMess.c_str());

    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to update device";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }

  httpClient.end();
}

void addDeviceHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String device_name = doc["device_name"].as<String>();
  String device_description = doc["device_description"].as<String>();
  int relay_id = doc["relay_id"].as<int>();

  Serial.println(device_name + ", " + device_description + ", " + relay_id);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/add_device.php");
  httpClient.addHeader("Content-Type", "application/json");
  
  String payload = "{\"device_name\":\"" + device_name + "\",\"device_description\":\"" + device_description + "\",\"relay_id\":" + String(relay_id) + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Device added successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to add device";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }
  httpClient.end();
}

void updateDeviceHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String device_name = doc["device_name"].as<String>();
  String device_description = doc["device_description"].as<String>();
  int relay_id = doc["relay_id"].as<int>();

  Serial.println(device_name + ", " + device_description + ", " + relay_id);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/update_device.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"device_name\":\"" + device_name + "\",\"device_description\":\"" + device_description + "\",\"relay_id\":" + String(relay_id) + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Device updated successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to update device";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }
  httpClient.end();
}

void addTimerHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String timer_name = doc["timer_name"].as<String>();
  String device_id = doc["device_id"].as<String>();
  String start_time = doc["start_time"].as<String>();
  int duration = doc["duration"].as<int>();
  String id = doc["id"].as<String>(); //new timer id

  Serial.println(timer_name + ", " + device_id + ", " + start_time + ", " + duration);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/add_timer.php");
  httpClient.addHeader("Content-Type", "application/json");
  
  String payload = "{\"timer_name\":\"" + timer_name + "\",\"device_id\":" + device_id + ",\"start_time\":\"" + start_time + "\",\"duration\":" + String(duration) + ",\"id\":" + id + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Timer added successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to add timer";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }
  httpClient.end();
}

void updateTimerHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String timer_id = doc["timer_id"].as<String>();
  String timer_name = doc["timer_name"].as<String>();
  String device_id = doc["device_id"].as<String>();
  String start_time = doc["start_time"].as<String>();
  int duration = doc["duration"].as<int>();

  Serial.println(timer_id + ", " + timer_name + ", " + device_id + ", " + start_time + ", " + duration);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/update_timer.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"timer_id\":" + timer_id + ",\"timer_name\":\"" + timer_name + "\",\"device_id\":" + device_id + ",\"start_time\":\"" + start_time + "\",\"duration\":" + String(duration) + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Timer updated successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to update timer";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }
  httpClient.end();
}

void controlTimerHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String timer_id = doc["timer_id"].as<String>();
  String state = doc["state"].as<String>();

  Serial.println(timer_id + ", " + state);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/control_timer.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"timer_id\":" + timer_id + ",\"state\":" + state + "}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Timer updated successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to update timer";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }

  httpClient.end();
}

void loginPostHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  JsonDocument doc;
  String jsonData = String((char*)data).substring(0, len);
  deserializeJson(doc, jsonData);

  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();

  Serial.println(username + ", " + password);

  HTTPClient httpClient;
  httpClient.begin(DATABASE_IP + "/login.php");
  httpClient.addHeader("Content-Type", "application/json");

  String payload = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";

  int httpCode = httpClient.POST(payload);

  JsonDocument response;
  String responseString;

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    response["message"] = "Login successfully";
    response["status"] = 200;
    serializeJson(response, responseString);
    request->send(200, "application/json", responseString);
  } else {
    response["message"] = "Fail to login";
    response["status"] = 400;
    serializeJson(response, responseString);
    request->send(400, "application/json", responseString);
  }

  httpClient.end();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  while(!Serial);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

  client.setCallback(getLastMessage);

  client.connect("ESP32Client", io_username, io_key);

  client.subscribe(last_feed);
  // Routing for web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/login.html", "text/html", false);
  });

  server.on("/home.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/home.html", "text/html", false, getDataForIndex);
  });

  server.on("/devices.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/devices.html", "text/html", false, getDataForDevices);
  });

  server.on("/timers.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/timers.html", "text/html", false, getDataForTimers);
  });

  server.on("/navbar.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/navbar.css", "text/css");
  });

  server.on("/homepage.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/homepage.css", "text/css");
  });

  server.on("/devices.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/devices.css", "text/css");
  });

  server.on("/timers.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/timers.css", "text/css");
  });

  server.on("/login.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/login.css", "text/css");
  });

  server.on("/pagination.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/pagination.js", "text/javascript");
  });

  server.on("/server.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/server.js", "text/javascript");
  });

  server.on("/An_Giang_farming.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/An_Giang_farming.jpg", "image/jpg");
  });
  //-------------APIs------------------//
  server.on("/control_device", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, controlDeviceHandler);

  server.on("/add_device", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, addDeviceHandler);

  server.on("/update_device", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, updateDeviceHandler);

  server.on("/delete_device", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = request->getParam("relay_id")->value();
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/delete_device.php?relay_id=" + id);
    int httpCode = httpClient.GET();

    JsonDocument response;
    String responseString;

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      response["message"] = "Device deleted successfully";
      response["status"] = 200;
      serializeJson(response, responseString);
      request->send(200, "application/json", responseString);
    } else {
      response["message"] = "Fail to delete device";
      response["status"] = 400;
      serializeJson(response, responseString);
      request->send(400, "application/json", responseString);
    }
    httpClient.end();
  });

  server.on("/add_timer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, addTimerHandler);

  server.on("/update_timer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, updateTimerHandler);

  server.on("/control_timer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, controlTimerHandler);

  server.on("/delete_timer", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = request->getParam("timer_id")->value();
    HTTPClient httpClient;
    httpClient.begin(DATABASE_IP + "/delete_timer.php?timer_id=" + id);
    int httpCode = httpClient.GET();

    JsonDocument response;
    String responseString;

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      response["message"] = "Timer deleted successfully";
      response["status"] = 200;
      serializeJson(response, responseString);
      request->send(200, "application/json", responseString);
    } else {
      response["message"] = "Fail to delete timer";
      response["status"] = 400;
      serializeJson(response, responseString);
      request->send(400, "application/json", responseString);
    }
    httpClient.end();
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, loginPostHandler);

  initWebSocket();

  server.begin();

  // xTaskCreate(maintainConenctionTask, "Maintain Connection Task", 8192, NULL, 0, NULL);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}