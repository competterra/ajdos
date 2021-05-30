#define  SENSOR_TEMPERATURE_DHT22
//#define  SENSOR_GAS_MQ135
#define  SENSOR_DUS_SDS011
#define  SENSOR_POWER_BATTERY

#include "adsensors.h"
#include "pins.h"
#include "dht22.h"
#include "mq135.h"
#include "battery.h"
#include "sds011.h"
#include "datahandler.h"

/****************************************************************************************/
class CAJDOS
/*--------------------------------------------------------------------------------------*/
{
  protected:
    CFLASHDATA*     m_lpFlash;
    CDATAHANDLER*   m_lpDataHandler;
    CWEBCLIENT*     m_lpWebClient;
    CDHT22*         m_lpTemperatureSensor;
    CBattery*       m_lpBatterySensor;
    CMQ135*         m_lpGasSensor;  
    CSDS011*        m_lpPMSensor;

  public:
      CAJDOS(CFLASHDATA* lpFlash);
      ~CAJDOS();

      void  createSensors();
      void  initSensors(CWEBCLIENT* lpWebClient);
      void  run();
};
/****************************************************************************************/


//=======================================================================================
CAJDOS::CAJDOS( CFLASHDATA* lpFlash )
//---------------------------------------------------------------------------------------
{
    m_lpFlash             = lpFlash;
    m_lpDataHandler       = new CDATAHANDLER( lpFlash );
    m_lpTemperatureSensor = NULL;
    m_lpBatterySensor     = NULL;
    m_lpGasSensor         = NULL;  
    m_lpPMSensor          = NULL;
    m_lpWebClient         = NULL;
    createSensors();  
}
//=======================================================================================
CAJDOS::~CAJDOS()
//---------------------------------------------------------------------------------------
{
  if (m_lpDataHandler)        delete m_lpDataHandler;         m_lpDataHandler       = NULL;
  if (m_lpTemperatureSensor)  delete m_lpTemperatureSensor;   m_lpTemperatureSensor = NULL;
  if (m_lpBatterySensor)      delete m_lpBatterySensor;       m_lpBatterySensor     = NULL;  
  if (m_lpGasSensor)          delete m_lpGasSensor;           m_lpGasSensor         = NULL;  
  if (m_lpPMSensor)           delete m_lpPMSensor;            m_lpPMSensor          = NULL;
  if (m_lpDataHandler)        delete m_lpDataHandler;         m_lpDataHandler       = NULL;
}
//=======================================================================================


//=======================================================================================
void CAJDOS::createSensors()
//---------------------------------------------------------------------------------------
{
#ifdef SENSOR_TEMPERATURE_DHT22
  if ( m_lpTemperatureSensor == NULL )
  {
     LOGLN("CREATE TEMPERATURE SENSOR");
     m_lpTemperatureSensor = new CDHT22();
  }
#endif  
  
#ifdef SENSOR_GAS_MQ135
  if ( m_lpGasSensor == NULL )
  {
     LOGLN("CREATE GAS SENSOR");
     m_lpGasSensor = new CMQ135();
  }
#endif
  
#ifdef SENSOR_DUS_SDS011  
  if ( m_lpPMSensor == NULL )
  {
     LOGLN("CREATE AIR QUALITY SENSOR");
     m_lpPMSensor = new CSDS011();
  }
#endif  

#ifdef SENSOR_POWER_BATTERY      
  if ( m_lpBatterySensor == NULL )
  {
     LOGLN("CREATE BATTERY SENSOR");
     m_lpBatterySensor = new CBattery();
  } 
#endif
}
//=======================================================================================
void CAJDOS::initSensors( CWEBCLIENT* lpWebClient )
//---------------------------------------------------------------------------------------
{  
  if ( m_lpTemperatureSensor ) m_lpTemperatureSensor->init();
  if ( m_lpGasSensor )         m_lpGasSensor->init();    
  if ( m_lpPMSensor )          m_lpPMSensor->init();      
  if ( m_lpBatterySensor )     m_lpBatterySensor->init();  
  m_lpWebClient = lpWebClient;
  m_lpDataHandler->initHandler( m_lpWebClient );
}
//=======================================================================================


//=======================================================================================
void CAJDOS::run()
//---------------------------------------------------------------------------------------
{
  float fTemperature;
  float fHumidity;
  m_lpDataHandler->addNewMeasures();   
  if ( m_lpTemperatureSensor )
  {    
      m_lpTemperatureSensor->wakeUp();
      fTemperature = m_lpTemperatureSensor->getValue(DHT_TEMPERATURE);
      fHumidity    = m_lpTemperatureSensor->getValue(DHT_HUMIDITY);
      LOGF("Temperature: %0.2f C\r\n",  fTemperature );
      LOGF("Humidity:    %0.2f %%\r\n", fHumidity );
      m_lpDataHandler->setTemperature( fTemperature, fHumidity );
  }  
  
  if ( m_lpGasSensor )
  {
      m_lpGasSensor->wakeUp( fTemperature, fHumidity );
      LOGF("Gas:     %0.2f \r\n", m_lpGasSensor->getValue() );
      m_lpDataHandler->setGas( m_lpGasSensor->getValue() );
  }
  
  if ( m_lpBatterySensor )
  {
      m_lpBatterySensor->wakeUp();
      LOGF("Battery:     %0.2f %%\r\n", m_lpBatterySensor->getValue() );
      m_lpDataHandler->setBattery( m_lpBatterySensor->getValue() );
  }

  if ( m_lpPMSensor )
  {
      m_lpPMSensor->wakeUp();
      LOGF("PM 2.5:     %0.2f %%\r\n", m_lpPMSensor->getValue(SDS_PM25) );
      LOGF("PM  10:     %0.2f %%\r\n", m_lpPMSensor->getValue(SDS_PM10) );
      m_lpDataHandler->setPM( m_lpPMSensor->getValue(SDS_PM25), m_lpPMSensor->getValue(SDS_PM10) );
  }  
  m_lpDataHandler->sendMeasures( true );
}
//=======================================================================================
