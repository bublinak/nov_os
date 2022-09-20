//#include "settings.h"                                                             // Settings file
//#include "version.h"                                                              // Version history file
#include <WiFi.h>
#include "src/PubSubClient/PubSubClient.h"

WiFiClient espClient;
PubSubClient client(espClient);

#include "src/Network/WiFi_handler.h"
WiFihandler wifi;

const char *default_ssid = "IoT Craft devboard"; 	// Default WiFi SSID
const char *default_pass = "12345678";			 	// Default WiFi PASS
const char *default_addr = "iotboard";			 	// Default board address (http://____.local/)

char server_addr[32] = "";					 		// 32-char array for MQTT broker domain
const char *mqtt_server = server_addr;		 		// Because of used library...
char prefix[] = "nvias/iotcraft";			 		// Prefix for the MQTT topic
char default_server[] = "broker.hivemq.com"; 		// Default MQTT server
int mqtt_port = 1883;						 		// MQTT broker port (default)

int serial_baudrate = 115200;						// HW Serial speed (overwritten by user setting)

#include "src/LuaWrapper/LuaWrapper.h" // Lua engine library
LuaWrapper lua;

#include "src/SerComm/terminal.h"					// Terminal app
terminal term;

#include <FS.h>		// Filesystem &
#include <SPIFFS.h> // SPIFFS drivers

#include <Preferences.h> // NVS flash interface
#include <nvs_flash.h>	 // NVS flash additional fonctionality lib.
Preferences os_settings;

#define FORCE_COLD_BOOT 1		  // Force cold boot
#define FORMAT_SPIFFS_IF_FAILED 0 // Auto-format SPIFFS if broken
#define OS_VERSION "Gemini-4"	  // Build OS

TaskHandle_t serComm;

#include "src/QueueArray/QueueArray.h" // CommandcoreQueue library
QueueArray<String> coreQueue;

void serialCommunication(void * pvParameters);

void callback(char *topic, byte *payload, unsigned int length)
{
	Serial.print("Message arrived in topic: ");
	Serial.println(topic);
	Serial.print("Message:");
	for (int i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println();
	Serial.println("-----------------------");
}

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Connecting to MQTT... ");
		// Attempt to connect
		if (client.connect(("IoTCraft-" + String(random(0xffff), HEX)).c_str()))
		{
			Serial.println("success.");
			// Subscribe
			client.subscribe("system/program");
		}
		else
		{
			Serial.print("fail, rc=");
			Serial.print(client.state());
			// vTaskDelay(1000/portTICK_PERIOD_MS);
		}
	}
}

void setup()
{
	delay(1000);
	os_settings.begin("os-pref", true);
	serial_baudrate = os_settings.getInt("serial-baudrate", serial_baudrate);
	Serial.begin(serial_baudrate);

	//=================  BOOT SELFCHECK =================
	if (!os_settings.getUChar("boot", 0) || FORCE_COLD_BOOT)
	{
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
		os_settings.putInt("mqtt_port", mqtt_port);

		os_settings.putInt("serial-baudrate", serial_baudrate);

		os_settings.end();

		wifi.firstSetup(default_ssid, default_pass, default_addr);
	}

	if (!SPIFFS.begin())
	{
		Serial.println("SPIFFS mount failed, formatting.\nPlease wait...");
		SPIFFS.format();
		vTaskDelay(16000 / portTICK_PERIOD_MS);
		Serial.println("Done.");
		return;
	}
	/*
	File file = SPIFFS.open("/script.lua", "w");
	file.print("print('Hello world!')\n\rdelay(1000)");
	file.close();
	*/
	delay(100);

	//--------------------------------------------------
	String temp_server = os_settings.getString("mqtt_server", default_server);
	for (int i = 0; i < temp_server.length(); i++)
	{
		server_addr[i] = temp_server.charAt(i);
	}

	//--------------------------------------------------

	// connecting to a WiFi network
	wifi.start();
	// connecting to a mqtt broker
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);

	delay(2000);
	//===================================================

	if (wifi.connectResult() == WIFI_DISCONNECTED)
	{
	}

	os_settings.end();
	xTaskCreatePinnedToCore(
      serialCommunication, /* Function to implement the task */
      "serComm", /* Name of the task */
      32768,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      &serComm,  /* Task handle. */
      0); /* Core where the task should run */
	delay(200);
}

void serialCommunication(void * pvParameters)
{
	
	Serial.print("NovOS CLI v.0.1 by nvias\n> ");
	vTaskDelay(200/portTICK_PERIOD_MS);
	while(true)
	{
		if (Serial.available() > 0)
		{
			String response = Serial.readString();
			Serial.print(response);
			response = term.processMessage(response);
			if(response.length() > 1)
			{
				Serial.print(response + "\n> ");
			}
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

void loop()
{
	while (!client.connected() && wifi.status() == WIFI_CONNECTED)
	{
		//reconnect();
		vTaskDelay(2000/portTICK_PERIOD_MS);
	}
	client.loop();
	//Serial.println(lua.Lua_dofile("/spiffs/script.lua"));
	client.publish("esp32/temperature", "Hello Friend :)");
	wifi.idle();
	vTaskDelay(10/portTICK_PERIOD_MS);
}
