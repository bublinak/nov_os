
#ifndef WIFI_HANDLER_H 
    #define WIFI_HANDLER_H 

	#define WIFI_DISCONNECTED 0
	#define WIFI_CONNECTED	1
	#define WIFI_STATION 2

	#define POST	1
	#define GET 2

    #include <WiFiClient.h>
    #include <WebServer.h>
    #include <DNSServer.h>
    #include <ESPmDNS.h>
    #include <Arduino.h>
    #include <WiFi.h>

	#include "../ElegantOTA/ElegantOTA.h"

    #include <FS.h>                                                                     // Filesystem &
    #include <SPIFFS.h>                                                                 // SPIFFS drivers

    #include <Preferences.h>                                                            // NVS flash interface
    #include <nvs_flash.h>                                                              // NVS flash additional fonctionality lib.

    const byte DNS_PORT = 53;


    class WiFihandler
    {
        public:
            WiFihandler();
            bool firstSetup(const char *ssid, const char *pass, const char *addr);
            bool start();
            bool idle();
			bool connectResult();
			bool status();
			int wget(String addr);


        private:


            bool softAPsetup();
    };
#endif
