
#define IOTWEBCONF_PASSWORD_LEN 65
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.

// ### Network and modbus #####################################################
// ############################################################################

// Name server
DNSServer dnsServer;

// Web server
WebServer server(80);

// IP of the SolarEdge inverter
IPAddress remote;

// ### IotWebConf #############################################################
// ############################################################################

// Is a reset required?
boolean needReset = false;

// Is wifi connected?
boolean connected = false;

// IotWebConf: Modifying the config version will probably cause a loss of the
// existig configuration. Be careful!
const char *CONFIG_VERSION = "1.0.0";

// IotWebConf: Access point SSID
const char *WIFI_AP_SSID = "TurnoutControl";

// IotWebConf: Default access point password
const char *WIFI_AP_DEFAULT_PASSWORD = "";

// IotWebConf: Method for handling access to / on web config
void handleRoot();

// IotWebConf: Called when connection to wifi was established
void wifiConnected();

// IotWebConf: Called when configuration saved
void configSaved();

// last known WiFi network state
iotwebconf::NetworkState lastNetWorkState = iotwebconf::NetworkState::OffLine;

IotWebConf iotWebConf(WIFI_AP_SSID, &dnsServer, &server, WIFI_AP_DEFAULT_PASSWORD, CONFIG_VERSION);

// Parameter group for turnout settings
IotWebConfParameterGroup groupTurnout = IotWebConfParameterGroup("groupTurnout", "Turnout Settings");

// Turnout 1 straight
char turnout1StraightParamValue[16];
IotWebConfNumberParameter turnout1StraightParam = IotWebConfNumberParameter("Turnout 1 straight", "turnout1Straight", turnout1StraightParamValue, 16, "700", "1..1500", "min='1' max='1500' step='1'");

// Turnout 1 turn
char turnout1TurnParamValue[16];
IotWebConfNumberParameter turnout1TurnParam = IotWebConfNumberParameter("Turnout 1 turn", "turnout1Turn", turnout1TurnParamValue, 16, "700", "1..1500", "min='1' max='1500' step='1'");

// ### Setup ##################################################################
// ############################################################################

void setup() {
    Serial.begin(115200); // Start the Serial communication to send messages to
                          // the computer
    delay(10);
    Serial.println("Starting up...");

    // -- Initializing the configuration.
    groupTurnout.addItem(&turnout1StraightParam);
    groupTurnout.addItem(&turnout1TurnParam);
    iotWebConf.addParameterGroup(&groupTurnout);

    iotWebConf.setWifiConnectionCallback(&wifiConnected);
    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setStatusPin(LED_BUILTIN);
    // iotWebConf.setConfigPin(D5);
    iotWebConf.init();

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });

    Serial.println("setup done");
}

// ### Main ###################################################################
// ############################################################################

void loop() {
    if (needReset) {
        // config changes require reset
        Serial.println("restart in 1 sec");
        delay(1000);
        ESP.restart();
    }

    if (!connected) {
    }

    if (connected) {
    }

    iotWebConf.doLoop();
}

void configSaved() {
    Serial.println("config saved");
    needReset = true;
}

void wifiConnected() {
    connected = true;
    Serial.println("wifi connected");

    // hack for getting hostname set correctly
    WiFi.hostname(iotWebConf.getThingName());
    WiFi.begin();
}

void handleRoot() {
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal()) {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" "
               "content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>IotWebConf 03 Custom Parameters</title></head><body>Hello world!";
    s += "<ul>";
    s += "<li>String param value: ";
    s += "NOT IN USE";
    s += "<li>Int param value: ";
    s += atoi(turnout1StraightParamValue);
    s += "<li>Float param value: ";
    s += atoi(turnout1TurnParamValue);
    s += "</ul>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
}