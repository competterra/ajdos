/*
 * 
 * AJDOS DHT22 Temperature sensor class v1.0
 * 
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 * dependencies: 
 *    <DHT22.h>   Adafruit temperature sensor library
 *    <EEPROM.h>  flash memory library
 *    hardware:   use a switch transistor for turn off power of the sensor by switching the GND level
 */
 
#include <DHT.h>
 
#define DHT_TEMPERATURE   0
#define DHT_HUMIDITY      1

//=======================================================================================
class CDHT22 : public ADSensor
//---------------------------------------------------------------------------------------
{
protected:  
   SensorParameters m_Params;
   int              m_nInputPin;
   int              m_nSwitchPin;
   DHT*             m_lpSensor;
   
public:   
   CDHT22();
   ~CDHT22();
   
   void             init();        
   bool             powerOn();
   void             powerOff();
   float            measuring( int nIndex );
   void             save();
};
//=======================================================================================


//---------------------------------------------------------------------------------------
CDHT22::CDHT22():ADSensor()
//---------------------------------------------------------------------------------------
{
   m_nInputPin  = DHT_DATA;
   m_nSwitchPin = DHT_SWITCH;
   m_lpSensor   = new DHT( m_nInputPin, DHT22 );
}
//---------------------------------------------------------------------------------------
CDHT22::~CDHT22()
{
  delete m_lpSensor;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CDHT22::init()
//---------------------------------------------------------------------------------------
{
  setDefaults( &m_Params );
  
  m_Params.lWarmingUp            = 10000; // 10 sec 
  m_Params.lWaitingTime          = 4000;  //  4 sec ( factory minimum is 2 sec)
  m_Params.nNumberOfMeasuring    =  4;
  m_Params.nNumberOfValues       =  2;    // temperature and humidity
  
  pinMode( m_nInputPin,  INPUT );
  pinMode( m_nSwitchPin, OUTPUT );
  digitalWrite( m_nSwitchPin, LOW );

  m_lpSensor->begin();
  ADSensor::init( &m_Params );
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
bool CDHT22::powerOn()
//---------------------------------------------------------------------------------------
// It is an empty method. Change this if you have to do other settings for the sensor or
// you could turn the sensor phisiclay on .
{
  digitalWrite( m_nSwitchPin, HIGH );
  return true;
}
//---------------------------------------------------------------------------------------
void CDHT22::powerOff()
{
  digitalWrite( m_nSwitchPin, LOW );
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
float CDHT22::measuring( int nIndex )
//---------------------------------------------------------------------------------------
{
  float fData = 0;
  switch ( nIndex )
  {
    case DHT_TEMPERATURE:
      if ( ! isnan( fData = m_lpSensor->readTemperature() ) ) 
      return fData;

    case DHT_HUMIDITY:
      if ( ! isnan( fData = m_lpSensor->readHumidity() ) ) 
      return fData;  
  }
  return m_lpParameters->fErrorValue;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CDHT22::save()
//---------------------------------------------------------------------------------------
{
  
}
