/**
 *  Definition of the config data structure for the master module and the class to persist it to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once
#include <Arduino.h>
#include <tinyTweetSecrets.h>   // This needs to be in a lib sub directory, obviously not in github...
#include <XEEPROMConfig.h>

#define CONFIG_VERSION 1
#define CONFIG_TYPE "tinyTweet"

#define HOSTNAME_MAX_LENGTH 50
#define DEFAULT_NTP_SERVER "0.europe.pool.ntp.org"

#define DEFAULT_STAT_PERIOD 1800000 // Half an hour. Should 0 be default ?

#define DEFAULT_GMT_HOUR_OFFSSET 2
#define DEFAULT_GMT_MIN_OFFSSET 0

struct configStruct:XEEPROMConfigDataStruct {
  // First member (version number) is inherited from XEEPROMConfigDataStruct   
  // Second member (config type) is inherited from XEEPROMConfigDataStruct
        
  // ntp server
  char ntpHostName[HOSTNAME_MAX_LENGTH + 1];

  char homeSsid[SSID_MAX_LENGTH + 1];
  char homePwd[PWD_MAX_LENGTH + 1];
    
  char apSsid[SSID_MAX_LENGTH + 1];
  char apPwd[PWD_MAX_LENGTH + 1];
  
  int8_t gmtHourOffset = DEFAULT_GMT_HOUR_OFFSSET;
  int8_t gmtMinOffset = DEFAULT_GMT_MIN_OFFSSET;
  
};

 
class ConfigClass:public XEEPROMConfigClass {
public:
  ConfigClass(unsigned int version);
  virtual void initFromDefault(void) override;
  
  void setHomeSsid(const char* ssid);
  void setAppHost(const char* appHost);
  void setAppHost(String appHost);
  void setNtpServer(String ntpServer);
  void setNtpServer(const char *ntpServer);
  void setHomeSsid(String ssid);
  void setHomePwd(const char* pwd);
  void setHomePwd(String pwd);
  void setApSsid(const char* ssid);
  void setApSsid(String ssid);
  void setApPwd(const char* pwd);
  void setApPwd(String pwd);
  char* getHomeSsid(void);
  char* getAppHost(void);
  char* getNtpServer(void);
  char* getHomePwd(void);
  char* getApSsid(bool force=false);
  char* getApPwd(bool force=false);
 
  bool isHomeWifiConfigured(void);
  bool isAPInitialized(void);
  void setGmtOffset(int8_t hour, int8_t min); 
  int8_t getGmtHourOffset(); 
  int8_t getGmtMinOffset(); 
  
protected:
  ConfigStruct* _getDataPtr(void);  
};
