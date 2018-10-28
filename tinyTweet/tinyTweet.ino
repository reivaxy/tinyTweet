/**
 *  tinyTweet 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

#include "config.h"

#include "initPageHtml.h"

#define TIME_STR_LENGTH 100

// Global object to store config
MasterConfigClass *config;
DisplayClass *oledDisplay;

ESP8266WebServer* server;
bool homeWifiConnected = false;
bool homeWifiFirstConnected = false;
NTPSyncEvent_t ntpEvent;
bool ntpEventToProcess = false;
bool ntpServerInitialized = false;
bool ntpTimeInitialized = false;
unsigned long elapsed200ms = 0;
unsigned long elapsed500ms = 0;
unsigned long elapsed2s = 0;

MDNSResponder mdns;
time_t timeNow = 0; 
time_t timeLastTimeDisplay = 0;
time_t timeLastWifiDisplay = 0;
time_t timeLastPing = 0;

int scl = 12;
int sda = 14;

// Handlers will work as long as these variables exists. 
static WiFiEventHandler wifiSTAGotIpHandler, wifiSTADisconnectedHandler;
                        
bool defaultAP = true;
bool displayAP = false;
bool ntpListenerInitialized = false;
String ipOnHomeSsid;

void setup() {
  
  WiFi.mode(WIFI_OFF);
  Serial.begin(9600);
  delay(100);
  config = new ConfigClass((unsigned int)CONFIG_VERSION);
  config->init();


  // Initialise the OLED display
  oledDisplay = new DisplayClass(0x3C, sda, scl);
  initDisplay();
  
  server = module->getServer();  
  server->on("/", HTTP_GET, [](){
    printHomePage();
  });
    
  // If Home wifi was configured previously, module should connect to Home Wifi.
  if(config->isHomeWifiConfigured()) {
    Serial.print(MSG_WIFI_CONNECTING_HOME);
    Serial.println(config->getHomeSsid());
    WiFi.mode(WIFI_STA);
    wifiSTAGotIpHandler = WiFi.onStationModeGotIP(onSTAGotIP); 
    wifiSTADisconnectedHandler = WiFi.onStationModeDisconnected(onSTADisconnected);
    WiFi.begin(config->getHomeSsid(), config->getHomePwd());
  } else {
    WiFi.mode(WIFI_AP);
    initSoftAP();
  } 
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    Serial.printf("NTP event: %d\n", event);
    ntpEventToProcess = true;
    ntpEvent = event;
  });
     
}

// Opens the Wifi network default access Point.
void initSoftAP() {
  Serial.print(MSG_WIFI_OPENING_AP);
  Serial.println(config->getApSsid());
  WiFi.softAP(config->getApSsid(), config->getApPwd());
  Serial.println(WiFi.softAPIP());
  wifiDisplay();
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
 
}

// Called when STA is connected to home wifi and IP was obtained
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
  ipOnHomeSsid = ipInfo.ip.toString();
  Serial.printf("Got IP on %s: %s\n", config->getHomeSsid(), ipOnHomeSsid.c_str());
  homeWifiConnected = true;
  homeWifiFirstConnected = true;
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  wifiDisplay();
}

void initNtp() {
  if(ntpServerInitialized) return;
  ntpServerInitialized = true;
  Serial.printf("Fetching time from %s\n", config->getNtpServer());
  NTP.begin(config->getNtpServer());
  NTP.setInterval(63, 7200);  // 63s retry, 2h refresh
  NTP.setTimeZone(config->getGmtHourOffset(), config->getGmtMinOffset());
}

void processNtpEvent() {
  if (ntpEvent) {
    Serial.print("NTP Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
    }
  else {
    Serial.print("Got NTP time: ");
    Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
    ntpTimeInitialized = true;
    NTP.setInterval(7200, 7200);  // 5h retry, 2h refresh. once we have time, refresh failure is not critical
  }
}
void onSTADisconnected(WiFiEventStationModeDisconnected event) {
  // Continuously get messages, so just output once.
//  if(homeWifiConnected) {
//    Serial.printf("Lost connection to %s, error: %d\n", event.ssid.c_str(), event.reason);
//    homeWifiConnected = false;
//    wifiDisplay();
//    NTP.stop();
//  }
}


void printHomePage() {
  // If init already done, display page saying so, otherwise display init page
  
  // TODO Disabled for now, need to be enabled !!
  if (false && config->isAPInitialized()) {
    Serial.println("Init done Page");
    module->sendText(MSG_INIT_ALREADY_DONE, 200);
  } else {
  
    char *page = (char *)malloc(strlen(initPage) + 10);
    sprintf(page, initPage, gsmEnabled ? "": "noGsm");
    module->sendHtml(page, 200);
    free(page);
    server->on("/initSave",  HTTP_POST, [](){
      Serial.println("Rq on /initSave");
      
      // TODO: /initSave might need to be disabled once done ?
      if(false && config->isAPInitialized()) {
        module->sendHtml(MSG_ERR_ALREADY_INITIALIZED, 403);
        return;
      }
      
      // TODO add some controls
      if (false && !server->hasArg("apSsid")) {
        module->sendText(MSG_ERR_BAD_REQUEST, 403);
        return;
      }
      String adminNumber = server->arg("admin");
      if (adminNumber.length() > 0) {
        if (adminNumber.length() < 10) {
          module->sendText(MSG_ERR_ADMIN_LENGTH, 403);
          return;
        }
        config->setAdminNumber(adminNumber);
        gsm.sendSMS(config->getAdminNumber(), "You are admin");  //
        oledDisplay->setLine(2, ""); 
        oledDisplay->setLine(2, MSG_INIT_DONE, true, false); 
      }
      
      // TODO: add checks in the config methods
      // Read and save new AP SSID 
      String apSsid = server->arg("apSsid");
      if (apSsid.length() > 0) {
        // TODO: add checks
        config->setApSsid(apSsid);
        wifiDisplay();
      }
      // Read and save the web app server      
      String appHost = server->arg("appHost");
      if (appHost.length() > 0) {
        // TODO: add checks
        config->setAppHost(appHost);
      }
            
      // Read and save the ntp host      
      String ntpHost = server->arg("ntpHost");
      if (ntpHost.length() > 0) {
        // TODO: add checks
        config->setNtpServer(ntpHost);
      }
      // Read and save new AP PWD 
      String apPwd = server->arg("apPwd");
      if( apPwd.length() > 0) {
        // Password need to be at least 8 characters
        if(apPwd.length() < 8) {
          module->sendText(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setApPwd(apPwd);
      }
            
      // Read and save home SSID 
      String homeSsid = server->arg("homeSsid");
      if (homeSsid.length() > 0) {
        config->setHomeSsid(homeSsid);
      }
      // Read and save home PWD 
      String homePwd = server->arg("homePwd");
      if( homePwd.length() > 0) {
        // Password need to be at least 8 characters
        if(homePwd.length() < 8) {
          module->sendText(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setHomePwd(homePwd);
      }
      
      
      // TODO: when GSM connected, send code, display confirmation page, 
      // and save once code confirmed
      // in the meantime, just save
      config->saveToEeprom();
      
      // New Access Point
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(config->getApSsid(), config->getApPwd());
      if(config->isHomeWifiConfigured()) {
        WiFi.begin(config->getHomeSsid(), config->getHomePwd());
      }
      
      printNumbers();
      module->sendText(MSG_INIT_DONE, 200);
      
    });     
  }
}

void initDisplay( void ) {
  char message[100];
  oledDisplay->setTitle(config->getName());
  if(gsmEnabled) oledDisplay->gsmIcon(BLINKING);  
  wifiDisplay();
  timeDisplay();
}

void timeDisplay() {
  oledDisplay->clockIcon(!ntpTimeInitialized);
  
  // TODO: if no Home wifi, no NTP, => test if  GSM enabled and use its time
  
  time_t millisec = millis();
  if(ntpServerInitialized && millisec > config->getDefaultAPExposition()) {
    oledDisplay->refreshDateTime(NTP.getTimeDateString().c_str());
  } else {
    char message[10];
    sprintf(message, "%d", millisec/1000);
    oledDisplay->refreshDateTime(message);
    
  }
}

void wifiDisplay() {
  char message[100];
  WifiType wifiType = AP;
  
  if(config->isAPInitialized()) {
    oledDisplay->setLine(1, "", NOT_TRANSIENT, NOT_BLINKING);
  } else {
    oledDisplay->setLine(1, MSG_INIT_REQUEST, NOT_TRANSIENT, BLINKING);
  }
    
  if(displayAP && homeWifiConnected) {
    strcpy(message, config->getHomeSsid());     
    strcat(message, " ");
    ipOnHomeSsid.getBytes((byte *)message + strlen(message), 50);
    oledDisplay->setLine(0, message);
  } else {
    strcpy(message, config->getApSsid());
    strcat(message, " ");
    IPAddress ipAddress = WiFi.softAPIP();
    ipAddress.toString().getBytes((byte *)message + strlen(message), 50);
    oledDisplay->setLine(0, message);
  }
  displayAP = !displayAP;
  
  bool blinkWifi = false;
  if (!homeWifiConnected && config->isHomeWifiConfigured()) {
    blinkWifi = true;
  }
  
  if(config->isHomeWifiConfigured()) {
    wifiType = AP_STA;
  } 
  oledDisplay->wifiIcon(blinkWifi, wifiType);
}



/*********************************
 * Main Loop
 *********************************/
void loop() {

  if(ntpEventToProcess) {
    ntpEventToProcess = false;
    processNtpEvent();
  }

  now();  // Needed to refresh the Time lib, so that NTP server is called
  // X seconds after reset, switch to custom AP if set
  if(defaultAP && (millis() > config->getDefaultAPExposition()) && config->isAPInitialized()) {
    defaultAP = false;
    initSoftAP();
  }
  
  // check if any new added agent needs to be renamed
  if(agentToRename != NULL) {
    agentCollection->renameOne(agentToRename);
    agentToRename = NULL;
  }
  
  // Check if any request to serve
  server->handleClient();
 
  // Let gsm do its tasks: checking connection, incomming messages, 
  // handler notifications...
  gsm.refresh();   
  
  // Display needs to be refreshed periodically to handle blinking
  oledDisplay->refresh();

  // Time on display should be refreshed every second
  // Intentionnally not using the value returned by now(), since it changes
  // when time is set.  
  timeNow = millis();
  
  if(timeNow - timeLastTimeDisplay >= 1000) {
    timeLastTimeDisplay = timeNow;
    timeDisplay();
  }
  if(timeNow - timeLastWifiDisplay >= 3500) {
    timeLastWifiDisplay = timeNow;
    // refresh wifi display every Xs to display both ssid/ips alternatively
    wifiDisplay();    
  }
  
  // TODO: ping every ? minute only 
  // TODO: should modules give their ping periodicity ? probably, with min to 15" ?     
  if(timeNow - timeLastPing >= 30000) {
    timeLastPing = timeNow; 
    agentCollection->ping();
  } 
  
  // Things to do only once after connection to internet.
  if(homeWifiFirstConnected) {
  // Init ntp   
    initNtp();
    homeWifiFirstConnected = false;
  }
  delay(20);
  
}