#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <base64.hpp>

#define       WC_NONE             0x00
#define       WC_PRESENT          0x01
#define       WC_READY            0x03

#define       MAX_TRIAL             10
#define       TRIAL_DELAY         1000
#define       TESTURL             "http://httpbin.org/get"

const char    mqtt_sayHello[]    = "ajdos/sensor/hello";
const char    mqtt_Data[]        = "ajdos/sensor/data";
const char    mqtt_Bye[]         = "ajdos/sensor/bye";

WiFiClient    g_WifiClient;  
PubSubClient  g_MqttClient( g_WifiClient );
unsigned char g_Base64[128];


char* encodePassword( char* lpPassword )
{
  unsigned char input[128];
  memset( &input, 0, sizeof( input ) );
  memcpy( &input, lpPassword, strlen( lpPassword ) );
  unsigned int base64_length = encode_base64( input, strlen(lpPassword), g_Base64);
  return (char*) &g_Base64[0];
}


//***************************************************************************************
class CWEBCLIENT
//--------------------------------------------------------------------------------------
{
  protected:            
      CFLASHDATA*   m_lpFLASH;
      int           m_nState;      
      
  public:
      
      CWEBCLIENT(CFLASHDATA* lpFLASH);
      ~CWEBCLIENT();

      bool  connect();
      void  closeConnection();
      bool  isConnected()   { return m_nState == WC_READY; }
      int   getState()      { return m_nState; }

      bool  HTTPTest();
      bool  MQTTTest();
      void  run();
      
      bool  beginSendData();
      void  sendMQTTHello( char* lpGMTTime );
      void  sendMQTTBye( char* lpGMTTime );      
      void  sendMQTTData( MEASURE* lpData, char* lpGMTTime );
      void  endSendData();
      
};
//***************************************************************************************

//=======================================================================================
CWEBCLIENT::CWEBCLIENT(CFLASHDATA* lpFLASH)
//---------------------------------------------------------------------------------------
// Constructor
{
    m_lpFLASH = lpFLASH;
    m_nState  = WC_PRESENT;
    if ( WiFi.status() == WL_NO_SHIELD )
    {
        m_nState = WC_NONE;
        LOGLN("WiFi shield not present");
    }      
}
//=======================================================================================
CWEBCLIENT::~CWEBCLIENT()
//---------------------------------------------------------------------------------------
{
  if ( isConnected() )
    closeConnection();    
}
//=======================================================================================


//=======================================================================================
bool CWEBCLIENT::connect()
//---------------------------------------------------------------------------------------
{
  if (m_nState == WC_NONE )
    return false;
  if (m_nState == WC_READY )
    return true;
    
  int nUsed = m_lpFLASH->getWifi()->nUsedConnection;
  if ( 1 < nUsed || nUsed < 0 )
  {
    LOGF("Wrong flash data. Used connection is %i\r\n", nUsed );
    return false;
  }
    
  m_nState = WC_PRESENT;
  int nConnectionCount = 0;
  while ( nConnectionCount < 2 )
  {    
    LOGF("Wifi connect to %s\r\n", m_lpFLASH->getWifi()->Connect[ nUsed ].SSID );
    if ( m_lpFLASH->getWifi()->Connect[ nUsed ].SSID[0] != '\0' )
    {
      WiFi.begin( m_lpFLASH->getWifi()->Connect[ nUsed ].SSID, m_lpFLASH->getWifi()->Connect[ nUsed ].Password );
      int nTrialCounter = 0;
      while ( WiFi.status() != WL_CONNECTED && nTrialCounter++ < MAX_TRIAL )
      {
        LOGF("--->%d", nTrialCounter );  
        delay(TRIAL_DELAY);
      }
      if ( WiFi.status() == WL_CONNECTED )
      {
        LOGLN("---> Connected");
        m_lpFLASH->getWifi()->nUsedConnection = nUsed;
        m_lpFLASH->getWifi()->Tested = true;;
        m_nState = WC_READY;
        return true;
      }
      m_lpFLASH->getWifi()->Tested = false;
      LOGLN("---> Not connected");
    }
    nUsed = ( nUsed == 1 ? 0 : 1 );
    ++nConnectionCount;
  }
  return false;
}
//=======================================================================================
void CWEBCLIENT::closeConnection()
//---------------------------------------------------------------------------------------
{
    WiFi.disconnect();
    LOGLN("Wifi Disconnected");
    m_nState = WC_PRESENT;
}
//=======================================================================================


//=======================================================================================
bool CWEBCLIENT::HTTPTest()
//---------------------------------------------------------------------------------------
{
  if ( m_nState != WC_READY )
    return false;
    
  HTTPClient* lpHttp = new HTTPClient();
  lpHttp->begin( TESTURL );
  int httpCode = lpHttp->GET();
  LOGF("HTTP test %s result %i\r\n", TESTURL, httpCode );
  lpHttp->end();
  delay(1000);
  delete lpHttp; 
  return (httpCode == 200);
}
//=======================================================================================


//=======================================================================================
bool CWEBCLIENT::MQTTTest()
//---------------------------------------------------------------------------------------
{
  if ( m_nState != WC_READY )
    return false;
  
  WiFiClient    wifiClient;  
  PubSubClient  mqttClient( wifiClient );
  bool          mqttConnected = false;
  int           nTrialCounter = 0;
    
  LOGF("MQTT connect %s:%d\r\n", m_lpFLASH->getMqtt()->IP, m_lpFLASH->getMqtt()->Port );
  mqttClient.setServer( m_lpFLASH->getMqtt()->IP, m_lpFLASH->getMqtt()->Port );
  while ( !(mqttConnected = mqttClient.connected()) &&  nTrialCounter++ < MAX_TRIAL )
  {
    if ( ! mqttClient.connect( m_lpFLASH->getSensor()->Name, m_lpFLASH->getMqtt()->User, m_lpFLASH->getMqtt()->Password ))
    {
        LOGF("===>%d(%i)", nTrialCounter, mqttClient.state() );  
        delay( TRIAL_DELAY );
    }
  }
  if ( mqttConnected )
  {
      LOGLN("MQTT connected");
      mqttClient.disconnect();
      return true;
  }
  return false;
}
//=======================================================================================



//=======================================================================================
void CWEBCLIENT::run()
//---------------------------------------------------------------------------------------
{
  m_lpFLASH->getApp()->TestResult = false;
  m_lpFLASH->getWifi()->Tested    = false;
  m_lpFLASH->getMqtt()->Tested    = false;

  if ( connect() )
  {
    delay( 1000 );
    if ( m_lpFLASH->getWifi()->Tested = HTTPTest() )
    {
       delay( 1000 );
       m_lpFLASH->getMqtt()->Tested = MQTTTest();
    }
    delay( 500 );
    closeConnection();
  }  
  m_lpFLASH->getApp()->TestResult = (m_lpFLASH->getWifi()->Tested & m_lpFLASH->getMqtt()->Tested);  
}
//=======================================================================================


//=======================================================================================
bool CWEBCLIENT::beginSendData()
//---------------------------------------------------------------------------------------
// connect to the WiFi and the MQTT server
{
  bool  mqttConnected = false;
  int   nTrialCounter = 0;
 
  if ( connect() )
  {
    LOGF("MQTT connect %s:%d\r\n", m_lpFLASH->getMqtt()->IP, m_lpFLASH->getMqtt()->Port );
    g_MqttClient.setServer( m_lpFLASH->getMqtt()->IP, m_lpFLASH->getMqtt()->Port );
    while ( !(mqttConnected = g_MqttClient.connected()) &&  nTrialCounter++ < MAX_TRIAL )
    {
      if ( ! g_MqttClient.connect( m_lpFLASH->getSensor()->Name, m_lpFLASH->getMqtt()->User, m_lpFLASH->getMqtt()->Password ))
      {
        LOGF("===>%d(%i)", nTrialCounter, g_MqttClient.state() );  
        delay( TRIAL_DELAY );
      }
    }
    if ( mqttConnected )
    {
      LOGLN("MQTT connected");
      return true;
    }
    closeConnection();
  }
  return false;
}
//=======================================================================================


//=======================================================================================
void CWEBCLIENT::endSendData()
//---------------------------------------------------------------------------------------
// disconnect from the MQTT server and close the WiFi connection too
{
  g_MqttClient.disconnect();
  closeConnection();  
}
//=======================================================================================




//=======================================================================================
void CWEBCLIENT::sendMQTTHello( char* lpGMTTime )
//---------------------------------------------------------------------------------------
{         
  char strMessage[1024];
  memset( &strMessage, 0, sizeof(strMessage) );
          
  snprintf( strMessage, sizeof(strMessage), "{\"name\":\"%s\", \"publickey\":\"%s\", \"owner\":\"%s\", \"station\":\"%s\", \"location\":\"POINT(%f %f %f m)\", \"cycletime\":%d, \"time\":\"%s\", \"crc\":\"%s\"}",
           m_lpFLASH->getSensor()->Name,
           encodePassword( m_lpFLASH->getSensor()->Password ),
           m_lpFLASH->getSensor()->OwnerId,
           m_lpFLASH->getSensor()->StationId,
           m_lpFLASH->getSensor()->Long,                                                 
           m_lpFLASH->getSensor()->Lat,
           m_lpFLASH->getSensor()->Alt,
           m_lpFLASH->getApp()->SleepTime,                         
           lpGMTTime,
           encodePassword( m_lpFLASH->getSensor()->HostPassword )
          );
          
  g_MqttClient.publish( mqtt_sayHello, strMessage );
  LOGF("MQTT %s > %s\r\n", mqtt_sayHello, strMessage );
  memcpy( m_lpFLASH->getOldSensor(), m_lpFLASH->getSensor(), sizeof(SENSORDATA) );
  delay(2000);                
}
//=======================================================================================


//=======================================================================================
void CWEBCLIENT::sendMQTTBye( char* lpGMTTime )
//---------------------------------------------------------------------------------------
{         
  char strMessage[1024];
  memset( &strMessage, 0, sizeof(strMessage) );
           
  snprintf( strMessage, sizeof(strMessage), "{\"name\":\"%s\", \"publickey\":\"%s\", \"time\":\"%s\"}",
                         m_lpFLASH->getOldSensor()->Name,
                         encodePassword( m_lpFLASH->getOldSensor()->Password ),
                         lpGMTTime 
                        );
                        
  g_MqttClient.publish( mqtt_Bye, strMessage );
  LOGF("MQTT %s > %s\r\n", mqtt_Bye, strMessage );
  memset( m_lpFLASH->getOldSensor(), 0, sizeof(SENSORDATA) );
  delay(2000);                
}
//=======================================================================================



//=======================================================================================
void CWEBCLIENT::sendMQTTData( MEASURE* lpData, char* lpGMTTime )
//---------------------------------------------------------------------------------------
{  
  char strMessage[1024];
  memset( &strMessage, 0, sizeof(strMessage) );
          
  snprintf( strMessage, sizeof(strMessage), "{\"name\":\"%s\", \"publickey\":\"%s\", \"location\":\"POINT(%f %f %f m)\", \"values\":{\"temperature\":%0.2f, \"humidity\":%0.2f, \"pm10\":%0.2f, \"pm25\":%0.2f, \"battery\":%0.2f }, \"time\":\"%s\" }",
            m_lpFLASH->getSensor()->Name,
            encodePassword( m_lpFLASH->getSensor()->Password ),
            m_lpFLASH->getSensor()->Long,   
            m_lpFLASH->getSensor()->Lat,
            m_lpFLASH->getSensor()->Alt,
                      ( (lpData->Mask & 1) == 1 ) ? lpData->Temperature : -1,
                      ( (lpData->Mask & 1) == 1 ) ? lpData->Humidity    : -1,
                      ( (lpData->Mask & 2) == 2 ) ? lpData->Pm10        : -1,
                      ( (lpData->Mask & 2) == 2 ) ? lpData->Pm25        : -1,
                      ( (lpData->Mask & 4) == 4 ) ? lpData->Battery     : -1,
                        lpGMTTime 
                    );                    
  g_MqttClient.publish( mqtt_Data, strMessage ); 
  LOGF("MQTT %s > %s\r\n", mqtt_Data, strMessage );                                
  delay(2000);                     
}
//=======================================================================================



#endif
