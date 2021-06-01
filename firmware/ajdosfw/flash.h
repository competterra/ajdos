/*
 * 
 * AJDOS FLASH Memory class v1.0
 * 
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 * dependencies: 
 * <EEPROM.h>
 */
//#ifndef FLASH_H
//#define FLASH_H
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#define MEM_SIZE                  4096
#define MAX_SAVED_MEASURES           5
#define DEFA_LAGTIME               100    // 100 sec
#define DEFA_SLEEPTIME             120    // 2 hours 
#define DEFA_PASSWORD       "12345678"

typedef struct _DAPPDATA
{
  size_t        Size;               // size of this structure
  char          UniqueID[32];       // generated uniquie ID of sensor
  int           CurrentMode;        // current runing mode
  int           ModeAfterReset;     // set this mode after the reset
  bool          Inicialized;        // true after setup done
  bool          TestResult;         // true if test connection result was good
  unsigned long SleepTime;          // ESP deep sleep in minutes (60000000 us = 1 min)
  unsigned long LagTime;            // Lag is a random number between 1..180 sec (1000000 ... 180000000 us)
  unsigned long LastSleepingTime;   // The last sleeping time in us
  time_t        LastNTPTime;        // Time of the last NTP connection 
  unsigned long NTPEllapsedTime;    // The ellapsed time since the last NTP connection
  unsigned int  MeasureItemCount;   // Number of stored measures (normaly is zero )
} APPDATA;

typedef struct _DSENSOR
{
  size_t        Size;               // size of this structure
  char          Name[64];           // unique name of the sensor
  char          Password[64];       // password for autentication
  char          OwnerId[64];        // owner id
  char          StationId[64];      // station id  
  char          HostPassword[64];   // HOST WIFI pasword in AP mode see in webserver.h   
  float         Long;               // Longitude
  float         Lat;                // Lattitude
  float         Alt;                // Altitude in meter
  float         Height;             // Ground Height above the Altitude
} SENSORDATA;

typedef struct _DWIFICONNECTION
{
  char          SSID[33];           // SSID;                          ssid
  char          Password[65];       // Password for SSID;             pwd 
} CONNECTION;

typedef struct _DWIFI
{
  size_t        Size;                // size of this structure
  int           nUsedConnection;     // 0 the default 1 the alternate WIFI connection (alternate WIFI connection can set via MQTT protocoll (to change the WIFI connection remotely)
  CONNECTION    Connect[2];  
  bool          Tested;              // TRUE if connection test result was OK 
} WIFIDATA;

typedef struct _DMQTT
{
  size_t        Size;               // size of this structure
  char          IP[32];             // Broker IP
  int           Port;               // Broker Port
  char          User[33];           // Broker User
  char          Password[33];       // Broker Password
  bool          Tested;             // TRUE if connection test result was OK
  bool          SendHello;          // TRUE if hello message have to send (When setup is changed or FLASH inicialized)
  bool          SendBye;            // TRUE if bye   message have to send (When setup is changed or FLASH inicialized and bye message was sended)
} MQTTDATA;


typedef struct _DMEASURE
{  
  size_t        Size;               // size of this structure
  time_t        Timestamp;          // Timestamp
  bool          Sended;             // TRUE if all data sended
  uint          Mask;               // 1 - temperature, Humidity, 2 - Pm25, Pm10, 4 - Battery, 8 - GAS, etc
  float         Temperature;
  float         Humidity;
  float         GasValue;
  float         Pm25;
  float         Pm10;
  float         Battery;  
} MEASURE;


typedef struct _FD
{   
    size_t      Size;               // size of this structure
    APPDATA     App;                // Application data
    SENSORDATA  Sensor;             // Sensor data 
    WIFIDATA    Wifi;               // WiFi data
    MQTTDATA    Mqtt;               // MQTT data    
    SENSORDATA  OldSensor;          // Saved sensor data (the bye message stores it )
} FLASHDATA;


//==============================================================
class CFLASHDATA
//--------------------------------------------------------------
{
protected:
    FLASHDATA     m_Mem;
    MEASURE       m_Measures[MAX_SAVED_MEASURES];   // Measured data
    bool          m_IsValid;
    bool          m_bFromFlash;
    String        m_strState;
    bool          m_bChanged;
    char          m_JSON[256];
public:
    CFLASHDATA();
   ~CFLASHDATA();

   void         Read();
   void         Write();
   void         Reset();
   void         ReadMeasure( int nIndex );
   void         WriteMeasure( int nIndex );
   
   bool         fromFlash()   { return m_bFromFlash;  }
   bool         isValid()     { return m_IsValid;     }
   APPDATA*     getApp()      { return &m_Mem.App;    }
   SENSORDATA*  getSensor()   { return &m_Mem.Sensor; }   
   WIFIDATA*    getWifi()     { return &m_Mem.Wifi;   }  
   MQTTDATA*    getMqtt()     { return &m_Mem.Mqtt;   }
   MEASURE*     getMeasures() { return &m_Measures[0];  }
   MEASURE*     getMeasure(int nIndex) { return &m_Measures[ nIndex ]; }
   SENSORDATA*  getOldSensor(){ return &m_Mem.OldSensor; }   
   
   char*        getSensorJSON();
   char*        getMeasuringJSON();
   char*        getWifiJSON();
   char*        getMqttJSON();
   char*        getAdminJSON();
   char*        getTestJSON();
  
   bool         IsChanged()             { return m_bChanged; }
   void         Change(bool bChg=true)  { m_bChanged=bChg;   }  
   void         Clear(); 
   bool         TestData();
   void         GenerateNewUniqueID();
   void         initMeasure( int nIndex );
};
//==============================================================


//==============================================================
CFLASHDATA::CFLASHDATA()
//--------------------------------------------------------------
//
{
    memset( &m_Mem,       0, sizeof( FLASHDATA ) );
    memset( &m_Measures,  0, sizeof( m_Measures ) );
    memset( &m_JSON,      0, sizeof( m_JSON )     );
    m_IsValid    = false;  
    m_bFromFlash = false;
    m_bChanged   = false;
    Read();
}
//==============================================================
CFLASHDATA::~CFLASHDATA()
//--------------------------------------------------------------
//
{
  
}
//==============================================================

//==============================================================
void CFLASHDATA::GenerateNewUniqueID()
//--------------------------------------------------------------
{
  String        mac( WiFi.macAddress() );
  unsigned long rnd = random(4096,65536);
  mac.replace(":","");
  sprintf(m_Mem.App.UniqueID, "%s-%s%X", mac.substring( 0, 8 ).c_str(), mac.substring( 8 ).c_str(), rnd );
}
//==============================================================


//==============================================================
void CFLASHDATA::Read()
//--------------------------------------------------------------
// Read and check EEPROM. Set the defaults if invalid.
//
{
    int nIndex;
    m_bFromFlash = true;
    EEPROM.begin( MEM_SIZE );
    EEPROM.get( 0, m_Mem );
    EEPROM.end();    
    if ( !(m_IsValid = (m_Mem.Size          == sizeof( FLASHDATA  )) &&
                       (m_Mem.App.Size      == sizeof( APPDATA    )) &&
                       (m_Mem.Sensor.Size   == sizeof( SENSORDATA )) &&
                       (m_Mem.Wifi.Size     == sizeof( WIFIDATA   )) &&
                       (m_Mem.Mqtt.Size     == sizeof( MQTTDATA   )) 
                       ))
    {
       Reset();
    }   
    for ( nIndex = 0; nIndex < m_Mem.App.MeasureItemCount; ++nIndex )
       ReadMeasure( nIndex );
    Change( false );
}
//==============================================================


//==============================================================
void CFLASHDATA::Write()
//--------------------------------------------------------------
// Write all data to the EEPROM. 
//
{
    TestData();
    EEPROM.begin( MEM_SIZE );
    EEPROM.put( 0, m_Mem );
    EEPROM.commit();
    EEPROM.end();    
    int nIndex;
    for ( nIndex = 0; nIndex < m_Mem.App.MeasureItemCount; ++nIndex )
       WriteMeasure( nIndex );    
    Change( false );
}
//==============================================================


//==============================================================
void CFLASHDATA::ReadMeasure( int nIndex )
//--------------------------------------------------------------
{
  int address = sizeof( FLASHDATA ) + nIndex*sizeof(MEASURE);
  EEPROM.begin( MEM_SIZE );
  EEPROM.get( address, m_Measures[ nIndex ] );
  EEPROM.end();
}
//==============================================================
void CFLASHDATA::WriteMeasure( int nIndex )
//--------------------------------------------------------------
//
{
    int address = sizeof( FLASHDATA ) + nIndex*sizeof(MEASURE);
    EEPROM.begin( MEM_SIZE );
    EEPROM.put( address, m_Measures[ nIndex ] );
    EEPROM.commit();
    EEPROM.end();    
}
//==============================================================



//==============================================================
void CFLASHDATA::Reset()
//--------------------------------------------------------------
{
    LOGLN("Inicialize the FLASH memory");        
        
    memset( &m_Mem.App, 0, sizeof( APPDATA ) );        
    m_Mem.App.Size           = sizeof( APPDATA );
    m_Mem.App.CurrentMode    = APPMODE_SETUP;
    m_Mem.App.ModeAfterReset = -1;
    m_Mem.App.Inicialized    = false;
    m_Mem.App.TestResult     = false;
    m_Mem.App.SleepTime      = DEFA_SLEEPTIME;
    m_Mem.App.LagTime        = DEFA_LAGTIME;
    GenerateNewUniqueID();      
    
    memset( &m_Mem.Sensor, 0, sizeof( SENSORDATA ) );        
    m_Mem.Sensor.Size = sizeof( SENSORDATA );        
    
    sprintf(m_Mem.Sensor.Name,      "SENSOR-%s",  m_Mem.App.UniqueID );
    sprintf(m_Mem.Sensor.OwnerId,   "OWNER-%s",   m_Mem.App.UniqueID );
    sprintf(m_Mem.Sensor.StationId, "STATION-%s", m_Mem.App.UniqueID );        
    strcpy(m_Mem.Sensor.HostPassword, DEFA_PASSWORD );
    strcpy(m_Mem.Sensor.Password,     DEFA_PASSWORD );
        
    memset( &m_Mem.Wifi, 0, sizeof( WIFIDATA ) );
    m_Mem.Wifi.Size = sizeof( WIFIDATA );  
        
    memset( &m_Mem.Mqtt, 0, sizeof( MQTTDATA ) );    
    m_Mem.Mqtt.Size = sizeof( MQTTDATA );
    m_Mem.Mqtt.SendHello = false;
    m_Mem.Mqtt.SendBye   = false;

    // memset( &m_Mem.Measures, 0, sizeof( MEASURES ) );
    m_Mem.Size = sizeof( FLASHDATA );
        
    Write(); 
}
//==============================================================


//==============================================================
void CFLASHDATA::initMeasure( int nIndex )
//--------------------------------------------------------------
{
  if (0 <= nIndex && nIndex < MAX_SAVED_MEASURES )
  {  
    memset( &m_Measures[nIndex], 0, sizeof( MEASURE ) );
    m_Measures[nIndex].Size = sizeof( MEASURE );
  }
}
//==============================================================


//==============================================================
// Return JSON object without starting and ending brackets.
//==============================================================
char* CFLASHDATA::getSensorJSON()
//--------------------------------------------------------------
{  
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"main\":{\"name\":\"%s\",\"password\":\"%s\",\"owner\":\"%s\",\"station\":\"%s\",\"lat\":\"%.8f\",\"long\":\"%.8f\",\"alt\":\"%.8f\",\"height\":\"%.8f\"}", m_Mem.Sensor.Name, m_Mem.Sensor.Password, m_Mem.Sensor.OwnerId, m_Mem.Sensor.StationId, m_Mem.Sensor.Lat, m_Mem.Sensor.Long, m_Mem.Sensor.Alt, m_Mem.Sensor.Height );
  return &m_JSON[0];
}
//==============================================================
char* CFLASHDATA::getMeasuringJSON()
//--------------------------------------------------------------
{
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"measuring\":{\"cycle\":\"%d\",\"lag\":\"%d\"}", m_Mem.App.SleepTime, m_Mem.App.LagTime );
  return &m_JSON[0];
}
//==============================================================
char* CFLASHDATA::getWifiJSON()
//--------------------------------------------------------------
{
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"wifi\":{\"ssid\":\"%s\",\"pwd\":\"%s\", \"tested\":%s }", m_Mem.Wifi.Connect[0].SSID, m_Mem.Wifi.Connect[0].Password, (m_Mem.Wifi.Tested ? "true" : "false") );
  return &m_JSON[0];
}
//==============================================================
char* CFLASHDATA::getMqttJSON()
//--------------------------------------------------------------
{
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"send\":{\"ip\":\"%s\",\"port\":\"%d\",\"user\":\"%s\",\"pwd\":\"%s\", \"tested\":%s }", m_Mem.Mqtt.IP, m_Mem.Mqtt.Port, m_Mem.Mqtt.User, m_Mem.Mqtt.Password, (m_Mem.Mqtt.Tested ? "true" : "false") );
  return &m_JSON[0];
}
//==============================================================
char* CFLASHDATA::getAdminJSON()
//--------------------------------------------------------------
{
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"admin\":{\"pwd\":\"%s\"}", m_Mem.Sensor.HostPassword );
  return &m_JSON[0];
}
//==============================================================
char* CFLASHDATA::getTestJSON()
//--------------------------------------------------------------
{
  memset( &m_JSON, 0, sizeof( m_JSON ) );
  snprintf( m_JSON, sizeof(m_JSON), "\"test\":{\"wifi\":\"%s\", \"mqtt\":\"%s\" }", (m_Mem.Wifi.Tested ? "OK" : "! Not tested or test failed"), (m_Mem.Mqtt.Tested ? "OK" : "! Not tested or test failed") );
  return &m_JSON[0];
}
//==============================================================


//==============================================================
void CFLASHDATA::Clear()
//--------------------------------------------------------------
{  
  memset( &m_Mem, 0, sizeof( m_Mem ) );
  Write();
}
//==============================================================


//==============================================================
bool CFLASHDATA::TestData()
//--------------------------------------------------------------
{ 
  getApp()->Inicialized = ( 
                            ( getSensor()->Lat != 0) && (getSensor()->Long != 0) &&
                            (strlen( getWifi()->Connect[0].SSID ) > 0) && (strlen( getWifi()->Connect[0].Password ) > 0 ) &&
                            (strlen( getMqtt()->IP ) > 0) && (strlen(getMqtt()->User) > 0) && (strlen(getMqtt()->Password) && getMqtt()->Port > 0)
                          );
  return ( getApp()->Inicialized );        
}
//============================================================== 

//#endif
