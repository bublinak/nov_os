
#ifndef WIFI_HANDLER_H 
    #define WIFI_HANDLER_H 

    #include <WiFiClient.h>
    #include <WebServer.h>
    #include <DNSServer.h>
    #include <ESPmDNS.h>
    #include <Arduino.h>
    #include <WiFi.h>

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


        private:


            bool softAP_setup();
    };
#endif
