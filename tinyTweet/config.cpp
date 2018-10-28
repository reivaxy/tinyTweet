/**
 *  Class to persist the master module configuration data structure to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include "config.h"

ConfigClass::ConfigClass(unsigned int version):XEEPROMConfigClass(version, CONFIG_TYPE, sizeof(MasterConfigStruct)) {

}

/**
 * Reset the config data structure to the default values.
 * This is done each time the data structure version is different from the one saved in EEPROM
 * NB: version and name are handled by base class 
 */
void ConfigClass::initFromDefault() {
  XEEPROMConfigClass::initFromDefault(); // handles version init
  MasterConfigStruct* configPtr = _getDataPtr();
  XUtils::safeStringCopy(configPtr->ntpHostName, DEFAULT_NTP_SERVER, HOSTNAME_MAX_LENGTH);
  configPtr->homeSsid[0] = 0;
  configPtr->homePwd[0] = 0;
  setApSsid(DEFAULT_APSSID);
  setApPwd(DEFAULT_APPWD);
  setHomeSsid("");
  setHomePwd("");
  setGmtOffset(DEFAULT_GMT_HOUR_OFFSSET, DEFAULT_GMT_MIN_OFFSSET);
}

void ConfigClass::setHomeSsid(const char* ssid) {
  XUtils::safeStringCopy(_getDataPtr()->homeSsid, ssid, SSID_MAX_LENGTH);
}
void ConfigClass::setHomeSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH + 1];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setHomeSsid(ssid);
}
void ConfigClass::setHomePwd(const char* pwd) {
  XUtils::safeStringCopy(_getDataPtr()->homePwd, pwd, PWD_MAX_LENGTH);
}
void ConfigClass::setHomePwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH + 1];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setHomePwd(pwd);
}

void ConfigClass::setApSsid(const char* ssid) {
  XUtils::safeStringCopy(_getDataPtr()->apSsid, ssid, SSID_MAX_LENGTH);
}
void ConfigClass::setAppHost(const char* appHost) {
  XUtils::safeStringCopy(_getDataPtr()->webAppHost, appHost, HOSTNAME_MAX_LENGTH);
}
void ConfigClass::setAppHost(String appHostString) {
  char appHost[HOSTNAME_MAX_LENGTH + 1];
  appHostString.toCharArray(appHost, (unsigned int)HOSTNAME_MAX_LENGTH);
  setAppHost(appHost);
}
void ConfigClass::setNtpServer(const char* ntpServer) {
  XUtils::safeStringCopy(_getDataPtr()->ntpHostName, ntpServer, HOSTNAME_MAX_LENGTH);
}
void ConfigClass::setNtpServer(String ntpServerString) {
  char ntpServer[HOSTNAME_MAX_LENGTH + 1];
  ntpServerString.toCharArray(ntpServer, (unsigned int)HOSTNAME_MAX_LENGTH);
  setNtpServer(ntpServer);
}

void ConfigClass::setApSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH + 1];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setApSsid(ssid);
}
void ConfigClass::setApPwd(const char* pwd) {
  XUtils::safeStringCopy(_getDataPtr()->apPwd, pwd, PWD_MAX_LENGTH);
}
void ConfigClass::setApPwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH + 1];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setApPwd(pwd);
}
char* ConfigClass::getHomeSsid(void) {
   return _getDataPtr()->homeSsid;
}
char* ConfigClass::getHomePwd(void) {
   return _getDataPtr()->homePwd;
}

char* ConfigClass::getAppHost(void) {
   return _getDataPtr()->webAppHost;
}

char* ConfigClass::getNtpServer(void) {
   return _getDataPtr()->ntpHostName;
}

char* ConfigClass::getApSsid(bool force) {
  if(force || millis() > getDefaultAPExposition())
    return _getDataPtr()->apSsid;
  else 
    return (char *)DEFAULT_APSSID;
}

char* ConfigClass::getApPwd(bool force) {
  if(force || millis() > getDefaultAPExposition())
    return _getDataPtr()->apPwd;
  else
    return (char *)DEFAULT_APPWD; 
}

void ConfigClass::setGmtOffset(int8_t hour, int8_t min) {
  _getDataPtr()->gmtHourOffset = hour;
  _getDataPtr()->gmtMinOffset = min;
} 
int8_t ConfigClass::getGmtHourOffset() {
  return _getDataPtr()->gmtHourOffset;
} 
int8_t ConfigClass::getGmtMinOffset() {
  return _getDataPtr()->gmtMinOffset;
}

// The home Wifi is configured if its ssid is not an empty string...  
bool ConfigClass::isHomeWifiConfigured() {
  if(*getHomeSsid() != 0) {
    return true;
  }
  return false;
}

// The AP is configured if its password is not the default password.
bool ConfigClass::isAPInitialized() {
  if(strcmp(_getDataPtr()->apPwd, DEFAULT_APPWD) != 0) {
    return true;
  }
  return false;
}

/**
 * Return the typed data structure object
 *
 */
MasterConfigStruct* ConfigClass::_getDataPtr(void) {
  return (MasterConfigStruct*)XEEPROMConfigClass::_getDataPtr();
}