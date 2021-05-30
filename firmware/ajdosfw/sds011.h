/*
 * 
 * AJDOS SDS011 Temperature sensor class v1.0
 * 
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 * dependencies: 
 *    <SdsDustSensor.h>   Adafruit SDS sensor library
 *    <EEPROM.h>          flash memory library
 *    hardware:   
 * TODO:    
 *    add humidity check (max 70%)
 *    
 *    https://www.hackster.io/taunoerik/using-sds011-dust-sensor-01f019
 */
 
#include "SdsDustSensor.h"
#include "SoftwareSerial.h"

#define SDS_PM25    0
#define SDS_PM10    1 


//=======================================================================================
class CSDS011 : public ADSensor
//---------------------------------------------------------------------------------------
{
protected:  
   SdsDustSensor*   m_lpSensor;
   SensorParameters m_Params;
   
   int              m_nRXPin;
   int              m_nTXPin;
   int              m_nSwitchPin;
   String           m_sFirmwareVersion;
   String           m_sReportingMode;
   String           m_sWorkingPeriod;
   float            m_pm25;
   float            m_pm10;
   bool             m_bData;
   
public:   
   CSDS011();
   ~CSDS011();
   
   void             init();
   void             setMode();
   void             pinSwitch( bool bOnOff = true )  { if ( m_nSwitchPin != -1 ) digitalWrite( m_nSwitchPin, (bOnOff ? HIGH : LOW ) ); }
   bool             powerOn();
   void             powerOff();
   float            measuring( int nIndex );
   void             save();
   String           getFirmwareVersion() { return m_sFirmwareVersion; };
   String           getReportingMode()   { return m_sReportingMode;   };
   String           getWorkingPeriod()   { return m_sWorkingPeriod;   };
};
//=======================================================================================


//---------------------------------------------------------------------------------------
CSDS011::CSDS011():ADSensor()
//---------------------------------------------------------------------------------------
{
   m_nRXPin     = SDS_RX;
   m_nTXPin     = SDS_TX;
   m_nSwitchPin = SDS_SWITCH;
   m_lpSensor   = new SdsDustSensor( m_nRXPin, m_nTXPin );
}
//---------------------------------------------------------------------------------------
CSDS011::~CSDS011()
{
  if ( m_lpSensor != NULL )
    delete m_lpSensor;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CSDS011::init()
//---------------------------------------------------------------------------------------
{
  setDefaults( &m_Params );
  
  m_Params.lWarmingUp            = 30000; // 30 sec 
  m_Params.lWaitingTime          = 4000;  //  4 sec ( factory minimum is 2 sec)
  m_Params.nNumberOfMeasuring    =  1;
  m_Params.nNumberOfValues       =  2;    
  m_Params.anRangeTemperature[0] = -10;
  m_Params.anRangeTemperature[1] =  50;
  m_Params.anRangeHumidity[0]    = MIN_HUMIDITY;
  m_Params.anRangeHumidity[1]    =  70;
  
  if ( m_nSwitchPin != -1 )
  {
      pinMode( m_nSwitchPin, OUTPUT );
      pinSwitch();
  }
  m_lpSensor->begin();
  delay( 500 );
  m_lpSensor->setQueryReportingMode();
  m_lpSensor->setContinuousWorkingPeriod();
  pinSwitch( false );
  ADSensor::init( &m_Params );
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CSDS011::setMode()
//---------------------------------------------------------------------------------------
{
  m_sFirmwareVersion = m_lpSensor->queryFirmwareVersion().toString();
  m_sReportingMode   = m_lpSensor->setQueryReportingMode().toString();
  m_sWorkingPeriod   = m_lpSensor->setContinuousWorkingPeriod().toString();
#ifdef DEBUG
  Serial.print("SDS firmware:");
  Serial.println( m_sFirmwareVersion );
  Serial.print("Reporting mode:");
  Serial.println( m_sReportingMode );
  Serial.print("Working period:");
  Serial.println( m_sWorkingPeriod );
#endif 
}


//---------------------------------------------------------------------------------------
bool CSDS011::powerOn()
//---------------------------------------------------------------------------------------
// It is an empty method. Change this if you have to do other settings for the sensor or
// you could turn the sensor phisiclay on .
{
  pinSwitch();
  WorkingStateResult state = m_lpSensor->wakeup(); 
  if (state.isWorking()) {
    setMode();  
#ifdef DEBUG        
    Serial.println("SDS011 sensor is working");  
#endif    
  }

  return true;
}
//---------------------------------------------------------------------------------------
void CSDS011::powerOff()
{
  WorkingStateResult state = m_lpSensor->sleep();
#ifdef DEBUG  
  if (state.isWorking()) {
      Serial.println("Problem with sleeping the SDS011 sensor.");
   } else {
      Serial.println("SDS011 sensor is sleeping");  
   }   
#endif   
  pinSwitch( false );
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
float CSDS011::measuring( int nIndex )
//---------------------------------------------------------------------------------------
{ 
  switch ( nIndex )
  { 
    case 0:
    {
      m_pm25  = m_lpParameters->fErrorValue;
      m_pm10  = m_lpParameters->fErrorValue;
      PmResult tData = m_lpSensor->queryPm();
      if ( tData.isOk() )
      {
          m_pm25 = tData.pm25;
          m_pm10 = tData.pm10;
          return m_pm25;
      }
      break;
    }  
    case 1:
    
        return m_pm10;
      break;  
  
  }
#ifdef DEBUG  
  Serial.print("SDS011 read error");
#endif  
  return m_lpParameters->fErrorValue;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CSDS011::save()
//---------------------------------------------------------------------------------------
{
  
}
