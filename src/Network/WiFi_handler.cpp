#include "WiFi_handler.h"

IPAddress apIP(8, 8, 8, 8);
IPAddress net_msk(255, 255, 255, 0);

DNSServer dns;
WebServer server(80);

Preferences wifi_settings;

char _ssid[32];
char _pass[32];
char _addr[32];

void handleRoot();
void handleNotFound();

WiFihandler::WiFihandler()
{

}

bool WiFihandler::firstSetup(const char *ssid, const char *pass, const char *addr)
{
    wifi_settings.begin("wifi-pref", false);
    wifi_settings.putString("ssid", String(ssid));
    wifi_settings.putString("pass", String(pass));
    wifi_settings.putString("ap-ssid", String(ssid));
    wifi_settings.putString("ap-pass", String(pass));
    wifi_settings.putString("mdns-addr", String(addr));

    wifi_settings.end();
    return true;
}

bool WiFihandler::start()
{
    wifi_settings.begin("wifi-pref", true);
    wifi_settings.getString("ssid").toCharArray(_ssid, 32);
    wifi_settings.getString("pass").toCharArray(_pass, 32);
    wifi_settings.getString("mdns-addr").toCharArray(_addr, 32);
    wifi_settings.end();
    WiFi.begin(_ssid, _pass);
    WiFi.waitForConnectResult();
    if (WiFi.status() != WL_CONNECTED) {
        softAP_setup();
    }



    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/generate_204", handleRoot);                                         //Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleRoot);                                               //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.onNotFound(handleNotFound);
    server.begin();                                                                 // Web server start
    Serial.println("HTTP server started");

    if (!MDNS.begin(_addr)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("MDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
    return true;
}

bool WiFihandler::softAP_setup()
{
    wifi_settings.begin("wifi-pref", true);
    wifi_settings.getString("ap-ssid").toCharArray(_ssid, 32);
    wifi_settings.getString("ap-pass").toCharArray(_pass, 32);
    wifi_settings.end();
    WiFi.softAPConfig(apIP, apIP, net_msk);
    WiFi.softAP(_ssid, _pass);
    delay(500); // Without delay I've seen the IP address blank
    //Serial.print("AP IP address: ");
    //Serial.println(WiFi.softAPIP());

    /* Setup the DNS server redirecting all the domains to the apIP */
    dns.setErrorReplyCode(DNSReplyCode::NoError);
    dns.start(DNS_PORT, "*", apIP);
    return true;
}

bool WiFihandler::idle()
{
    dns.processNextRequest();
    //HTTP
    server.handleClient();
    return true;
}

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(_addr) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Handle root or redirect to captive portal */
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>HELLO WORLD!!</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + String(WiFi.softAPIP()) + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + String(_ssid) + F("</p>");
  }
  Page += F(
            "<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
}

void handleNotFound() {
    if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
        return;
    }
    String message = F("File Not Found\n\n");
    message += F("URI: ");
    message += server.uri();
    message += F("\nMethod: ");
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += server.args();
    message += F("\n");

    for (uint8_t i = 0; i < server.args(); i++) {
        message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
    }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(404, "text/plain", message);
}