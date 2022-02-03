//#include "settings.h"                                                             // Settings file
//#include "version.h"                                                              // Version history file
#include <WiFi.h>
#include "src/PubSubClient/PubSubClient.h"

WiFiClient espClient;
PubSubClient client(espClient);

#include "src/Network/WiFi_handler.h"
WiFihandler wifi;

const char *default_ssid = "IoT Craft devboard";                                    // Default WiFi SSID
const char *default_pass = "12345678";                                              // Default WiFi PASS
const char *default_addr = "iotboard";                                              // Default board address (http://____.local/)

char server_addr[32] = "";                                                          // 32-char array for MQTT broker domain
const char *mqtt_server = server_addr;                                              // Because of used library...
char prefix[] = "nvias/iotcraft";                                                   // Prefix for the MQTT topic
char default_server[] = "broker.hivemq.com";                                        // Default MQTT server
int mqtt_port = 1883;                                                               // MQTT broker port (default)

#include "src/LuaWrapper/LuaWrapper.h"                                              // Lua engine library
LuaWrapper lua;

#include <FS.h>                                                                     // Filesystem &
#include <SPIFFS.h>                                                                 // SPIFFS drivers

#include <Preferences.h>                                                            // NVS flash interface
#include <nvs_flash.h>                                                              // NVS flash additional fonctionality lib.
Preferences os_settings;

#define FORCE_COLD_BOOT 1                                                           // Force cold boot
#define FORMAT_SPIFFS_IF_FAILED 0                                                   // Auto-format SPIFFS if broken
#define OS_VERSION "Gemini-2"                                                       // Build OS

//#include "src/QueueArray.h"                                                       // CommandcoreQueue library
//QueueArray <String>coreQueue;





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
    os_settings.begin("os-pref", true);
    Serial.begin(115200);

//=================  BOOT SELFCHECK =================
    if(!os_settings.getUChar("boot", 0) || FORCE_COLD_BOOT){
        // COLD BOOT HANDLER
        // UPGRADE PROCEDURE NOT IMPLEMENTED YET
        Serial.println("Cold boot");
        os_settings.end();
        nvs_flash_erase();
        nvs_flash_init();
        os_settings.begin("os-pref", false);
        os_settings.putUChar("boot", 1);
        os_settings.putString("version", OS_VERSION);

        os_settings.putString("mqtt_server", String(default_server));

        os_settings.end();

        wifi.firstSetup(default_ssid, default_pass, default_addr);
    }
    coreQueue = xQueueCreate(16, sizeof(Message));

    wifi.start();

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        SPIFFS.format();
        delay(500);
        return;
    }
    delay(100);
    File file = SPIFFS.open("/script.lua", "w");
    file.print("print('Hello world!')\n\rdelay(1000)");
    file.close();

    for(int i=0; i<sizeof(default_server)/sizeof(char); i++){
        server_addr[i] = default_server[i];
    }
    // connecting to a WiFi network
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
      vTaskDelay(5000/portTICK_PERIOD_MS);
      wifi.idle();
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
    wifi.idle();
    vTaskDelay(portTICK_PERIOD_MS);
}
