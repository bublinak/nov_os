//#include "settings.h"                                         // Settings file
//#include "version.h"                                          // Version history file
#include <WiFi.h>
#include "src/PubSubClient.h"

#include "FS.h"
#include <SPIFFS.h>
#include <esp_spiffs.h>
#include <Preferences.h>                                        // NVS flash interface
#include <nvs_flash.h>                                          // NVS flash additional fonctionality lib.
Preferences settings;

// WiFi
const char *ssid = "WiFi-HSP"; // Enter your WiFi name
const char *password = "WiFi-HSP-ASUS";  // Enter WiFi password

// MQTT Broker
char server[32];
const char *mqtt_server = server;
const char *topic = "esp32/test";
//const char *mqtt_username = "emqx";
//const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


#define FORCE_COLD_BOOT 1                                       // Force cold boot
#define FORMAT_SPIFFS_IF_FAILED 1                               // Auto-format SPIFFS if broken
#define OS_VERSION "Gemini-1"                                   // Build OS

//#include "src/QueueArray.h"                                   // CommandcoreQueue library
//QueueArray <String>coreQueue;
#include "src/LuaWrapper.h"


LuaWrapper lua;

typedef struct Message 
{
    int pid;        // PID - Program ID (01-Commpy, 02-Netty, 03-Perry, 04-Porty) 
    int status;     // Program status (00-Stopped, 01-Idle, 02-Working, 03-Error, 04-Crash)
    char body[32];  // The message for core
}   Message;
Message msg;
static QueueHandle_t coreQueue;

void setup()
{
    delay(1000);
    settings.begin("os-pref", true);
    Serial.begin(115200);

//=================  BOOT SELFCHECK =================
    if(!settings.getUChar("boot", 0) || FORCE_COLD_BOOT){
        // COLD BOOT HANDLER
        // UPGRADE PROCEDURE NOT IMPLEMENTED YET
        Serial.println("Cold boot");
        settings.end();
        nvs_flash_erase();
        nvs_flash_init();
        settings.begin("os-pref", false);
        settings.putUChar("boot", 1);
        settings.putString("version", OS_VERSION);
        settings.end();
    }
    coreQueue = xQueueCreate(16, sizeof(Message));

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    File file = SPIFFS.open("/script.lua", "w");
    file.print("print('Hello world!')\n\rdelay(1000)");
    file.close();

    char the_addr[] = "broker.hivemq.com";
    for(int i=0; i<sizeof(the_addr)/sizeof(char); i++){
        server[i] = the_addr[i];
    }
    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    delay(2000);
//===================================================

}

void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPiotClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    Serial.println(lua.Lua_dofile("/spiffs/script.lua"));
    client.publish("esp32/temperature", "Hello Friend :)");
    vTaskDelay(portTICK_PERIOD_MS);
}
