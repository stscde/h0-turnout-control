
#define IOTWEBCONF_PASSWORD_LEN 65
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.

// ### Network ################################################################
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

// ### Turnout values #########################################################
// ############################################################################

// number of turnouts supported
const int TURNOUT_COUNT = 10;

// parsed turnout values from parameters, 0 = straight value 1 = divergent value
int turnoutParamValues[TURNOUT_COUNT][2];

// Values for turnouts in straight direction, comma separated, first value is turnout 1
char turnoutStraightParamValue[40] = {""};
IotWebConfTextParameter turnoutStraightParam = IotWebConfTextParameter("Turnout straight values", "turnoutStraightValues", turnoutStraightParamValue, 40, "101,102,103,104,105,106,107,108,109,110");

// Values for turnouts in divergent direction, comma separated, first value is turnout 1
char turnoutDivParamValue[40] = {""};
IotWebConfTextParameter turnoutDivParam = IotWebConfTextParameter("Turnout divergent values", "turnoutDivValues", turnoutDivParamValue, 40, "201,202,203,204,205,206,207,208,209,210");

/**
 * Parse turnout values from straight and divergent parameter values
 */
void parseTurnoutValues();

// ### Setup ##################################################################
// ############################################################################

void setup() {
    Serial.begin(115200); // Start the Serial communication to send messages to
                          // the computer
    delay(10);
    Serial.println("Starting up...");

    // -- Initializing the configuration.
    groupTurnout.addItem(&turnoutStraightParam);
    groupTurnout.addItem(&turnoutDivParam);

    Serial.println("turnouts added");
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

    //   iotWebConf.getSystemParameterGroup()->applyDefaultValue();
    //   iotWebConf.saveConfig();

    parseTurnoutValues();
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
    s += "<title>Turnout configuration</title></head><body>Turnout settings";
    s += "<br>";

    s += "<table>";
    s += "  <tr>";
    s += "    <th>Turnout #</th>";
    s += "    <th>Straight Value</th>";
    s += "    <th>Divergent Value</th>";
    s += "  </tr>";

    for (int t = 0; t < TURNOUT_COUNT; t++) {
        s += "  <tr>";

        char buff[5];
        sprintf(buff, "%i", (t + 1));
        s += "    <td>";
        s += buff;
        s += "    </td>";

        sprintf(buff, "%i", turnoutParamValues[t][0]);
        s += "    <td>";
        s += buff;
        s += "    </td>";

        sprintf(buff, "%i", turnoutParamValues[t][1]);
        s += "    <td>";
        s += buff;
        s += "    </td>";

        s += "  </tr>";
    }

    s += "</table>";

    s += "<br>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
}

void parseTurnoutValues() {

    // parse straight values
    Serial.print("parse straight values: ");
    Serial.println(turnoutStraightParamValue);

    char straightValuesCopy[40];
    strcpy(straightValuesCopy, turnoutStraightParamValue);

    char *ptr = strtok(straightValuesCopy, ",");
    int i = 0;
    while (ptr) {
        turnoutParamValues[i][0] = atoi(ptr);
        ptr = strtok(NULL, ",");
        i++;
    }

    // parse divergent values
    Serial.print("parse divergent values: ");
    Serial.println(turnoutDivParamValue);

    char divValuesCopy[40];
    strcpy(divValuesCopy, turnoutDivParamValue);

    ptr = strtok(divValuesCopy, ",");
    i = 0;
    while (ptr) {
        turnoutParamValues[i][1] = atoi(ptr);
        ptr = strtok(NULL, ",");
        i++;
    }

    // print parsed values for debug reasons
    for (int t = 0; t < TURNOUT_COUNT; t++) {
        Serial.print("turnout: ");
        Serial.print((t + 1));
        Serial.print(": straight: ");
        Serial.print(turnoutParamValues[t][0]);
        Serial.print(", divergent: ");
        Serial.println(turnoutParamValues[t][1]);
    }
}
