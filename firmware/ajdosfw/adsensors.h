/*
 * 
 * AJDOS Sensor v1.0
 * interface and base class
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 */


#define MIN_TEMPERATURE      -200
#define MAX_TEMPERATURE       200
#define MIN_HUMIDITY            0
#define MAX_HUMIDITY          100

#define DEF_WARMINGTIME      2000                                                         // 2sec
#define DEF_WAITINGTIME      5000                                                         // 5sec
#define DEF_COUNTOFMEASURING   10                                                         // 10x
#define DEF_VALUES              1     
#define DEF_ERRORVALUE         -1.

#define STATE_SLEEP             0
#define STATE_CHECKING          1
#define STATE_WARMING           2
#define STATE_MEASURING         3
#define STATE_WAITING           4



typedef struct __SensorParameters
{
     int    anRangeTemperature[2];                                                        // min and max temperature used by checkEnvironment()
     int    anRangeHumidity[2];                                                           // min and max hummidity used by checkEnvironment()
     long   lWarmingUp;                                                                   // delay in ms for warming
     long   lWaitingTime;                                                                 // delay in ms between two measuring
     int    nNumberOfMeasuring;                                                           // number of measuring to do
     int    nNumberOfValues;                                                              // number of measured values;
     float  fErrorValue;                                                                  // error value when measuring can't possible;  
} SensorParameters;



/****************************************************************************************/
class ISensor
/*--------------------------------------------------------------------------------------*/
{
  protected:
  
      SensorParameters*  m_lpParameters;                                                  // sensor parameters 
      int                m_nState;                                                        // state of the sensor 
      int                m_nCounter;                                                      // number of current measuring
      int                m_nTotalCounter;                                                 // number of wakeup
      int                m_nMissedCounter;                                                // number of misses measuring     
      float*             m_lpValues;                                                      // average value of the measured parameters
     
  public:
      
      ISensor() {};
      ~ISensor() {};
      
      virtual void      init( SensorParameters* lpParameters = NULL ) {};                 // initialize the sensor parameters
      virtual void      setState( int nState ) {};                                        // set the sensor state
      virtual int       getState() {};                                                    // return the current state
      virtual bool      wakeUp( float curTemperature, float curHumidity ) {};             // start sensing mode
      virtual void      gotoSleep() {};                                                   // set the sensors in sleep mode (call turn off in default)
      virtual bool      powerOn() {};                                                     // turn on the sensor
      virtual void      warming() {};                                                     // enters standby mode until the senror warms up
      virtual void      waiting() {};                                                     // set waiting mode between to measurements
      virtual void      powerOff() {};                                                    // turn off the sensor
      virtual float     measuring( int index ) {};                                        // call measuring methodes
      virtual void      calculating( float* lpData ) {};                                  // calculate the average value from the measured data.
      virtual void      load() {};                                                        // load data from ESP FLASH memory
      virtual void      save() {};                                                        // save data to ESP FLASH memory
      virtual float     getValue(int nIndex = 0 ) {};                                     // get the measured data indexed by nIndex parameter
      virtual long      getMeasuringTime() {};                                            // get the time of the total measuring cycle in ms
      
};
/****************************************************************************************/



/****************************************************************************************/
class ADSensor : public ISensor
/*--------------------------------------------------------------------------------------*/
{
protected:
     SensorParameters*  m_lpParameters;
     int                m_nState;
     int                m_nCounter;                                                       // number of current measuring
     int                m_nTotalCounter;                                                  // number of wakeup
     int                m_nMissedCounter;                                                 // number of misses measuring     
     float*             m_lpValues;                                                       // average value of the measured parameters
     bool               m_bOwnParameters;                                                 // true if memory allocated by the init method
       
public:
      ADSensor();
      ~ADSensor();

      virtual void      setDefaults( SensorParameters* lpParameters );
      virtual void      init( SensorParameters* lpParameters = NULL );      
      virtual void      setState( int nState );
      virtual int       getState();
      virtual bool      wakeUp( float curTemperature, float curHumidity );
      virtual void      gotoSleep();
      virtual bool      powerOn();
      virtual void      warming();
      virtual void      waiting();
      virtual void      powerOff();
      virtual float     measuring( int index );
      virtual void      calculating( float* lpData );
      virtual void      load() {};
      virtual void      save() {};
      virtual float     getValue(int nIndex = 0 );
      virtual long      getMeasuringTime();
};
/****************************************************************************************/


//=======================================================================================
ADSensor::ADSensor()
//---------------------------------------------------------------------------------------
// constructor
{
    m_lpParameters   = NULL;
    m_lpValues       = NULL;
    m_nCounter       = 0;
    m_nTotalCounter  = 0;
    m_nMissedCounter = 0;  
    m_bOwnParameters = false;
}
//=======================================================================================
ADSensor::~ADSensor()
//---------------------------------------------------------------------------------------
// destructor
{
  if ( m_lpParameters && m_bOwnParameters ) free( m_lpParameters ); m_lpParameters = NULL;
  if ( m_lpValues )                         free( m_lpValues );     m_lpValues     = NULL;
}
//=======================================================================================



//---------------------------------------------------------------------------------------
void ADSensor::setDefaults( SensorParameters* lpParameters )
//---------------------------------------------------------------------------------------
{
  lpParameters->anRangeTemperature[0] = MIN_TEMPERATURE;
  lpParameters->anRangeTemperature[1] = MAX_TEMPERATURE;
  lpParameters->anRangeHumidity[0]    = MIN_HUMIDITY;;
  lpParameters->anRangeHumidity[1]    = MAX_HUMIDITY;
  lpParameters->lWarmingUp            = DEF_WARMINGTIME;
  lpParameters->lWaitingTime          = DEF_WAITINGTIME;
  lpParameters->nNumberOfMeasuring    = DEF_COUNTOFMEASURING;
  lpParameters->nNumberOfValues       = DEF_VALUES;
  lpParameters->fErrorValue           = DEF_ERRORVALUE;  
}
//---------------------------------------------------------------------------------------
void ADSensor::init( SensorParameters* lpParameters )
//---------------------------------------------------------------------------------------
{ 
  m_bOwnParameters = false;
  if ( lpParameters == NULL )
  {
      m_bOwnParameters = true;
      m_lpParameters   = (SensorParameters*) malloc( sizeof(SensorParameters) );
      memset( m_lpParameters, 0, sizeof(SensorParameters) );
      setDefaults( m_lpParameters );      
  }
  else
     m_lpParameters = lpParameters;
     
  m_lpValues = (float*) malloc( sizeof( float)*m_lpParameters->nNumberOfValues );
  memset( m_lpValues, 0, sizeof( float)*m_lpParameters->nNumberOfValues );
  m_nCounter = 0;
  setState( STATE_SLEEP );
}
//---------------------------------------------------------------------------------------
 void ADSensor::setState( int nState )
//---------------------------------------------------------------------------------------
{
   m_nState = nState;
}
 //---------------------------------------------------------------------------------------
int  ADSensor::getState()
{
  return m_nState;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
bool ADSensor::powerOn()
//---------------------------------------------------------------------------------------
// It is an empty method. Change this if you have to do other settings for the sensor or
// you could turn the sensor phisiclay on .
{  
  return true;
}
//---------------------------------------------------------------------------------------
void ADSensor::powerOff()
{
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
bool ADSensor::wakeUp( float curTemperature = 0, float curHumidity = 0 )
//---------------------------------------------------------------------------------------
// 
{
  
   bool bResult = false;  
   ++m_nTotalCounter;
   setState( STATE_CHECKING );
   if ( ( m_lpParameters->anRangeTemperature[0] <= curTemperature and curTemperature <= m_lpParameters->anRangeTemperature[1]) and
        ( m_lpParameters->anRangeHumidity[0]    <= curHumidity    and curHumidity    <= m_lpParameters->anRangeHumidity[1]) and 
        powerOn() )
   {
        setState( STATE_WARMING );
        warming();
        
        int    nSizeOfValues = sizeof( float ) * m_lpParameters->nNumberOfMeasuring * m_lpParameters->nNumberOfValues;
        float* lpValues      = (float*) malloc( nSizeOfValues );
        float* lpValue       = NULL;        
        memset( lpValues, 0, nSizeOfValues );
        
        for ( m_nCounter=0; m_nCounter<m_lpParameters->nNumberOfMeasuring; ++m_nCounter )
        {
          setState( STATE_MEASURING );
          for ( int nValues=0; nValues<m_lpParameters->nNumberOfValues; ++nValues )
          {
            lpValue  = lpValues;
            lpValue += m_nCounter*m_lpParameters->nNumberOfValues + nValues;    
            *lpValue = measuring( nValues );
          }
          
          if ( m_nCounter < m_nCounter < m_lpParameters->nNumberOfMeasuring-1 )
          {
            setState( STATE_WAITING );
            waiting();            
          }
        }
        calculating( lpValues );  
              
        free( lpValues );
        powerOff();        
        bResult = true;  
   }
   else
      ++m_nMissedCounter;
   save();      
   setState( STATE_SLEEP );   
   return bResult;      
}
//---------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------
void ADSensor::warming()
//---------------------------------------------------------------------------------------
// waiting for sensor is ready
{    
    delay( m_lpParameters->lWarmingUp );
}
//---------------------------------------------------------------------------------------
void ADSensor::waiting()
//---------------------------------------------------------------------------------------
// waiting for sensor is ready
{    
    delay( m_lpParameters->lWaitingTime );
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
void ADSensor::gotoSleep()
//---------------------------------------------------------------------------------------
{
  powerOff();
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
float ADSensor::measuring( int nIndex )
//---------------------------------------------------------------------------------------
{
  switch (nIndex)
  {
    case 0:
      return 0.;
      break;
    default:
      return m_lpParameters->fErrorValue;  
  }
}
//---------------------------------------------------------------------------------------
void ADSensor::calculating( float* lpData )
//---------------------------------------------------------------------------------------
{
 
  int*   lpCounts = (int*) malloc( sizeof( int ) * m_lpParameters->nNumberOfValues );
  int*   lpCount  = NULL;
  float* lpCurrent= NULL;
  float* lpCalc   = NULL;
  
  memset( lpCounts,   0, sizeof( int   ) * m_lpParameters->nNumberOfValues );
  memset( m_lpValues, 0, sizeof( float ) * m_lpParameters->nNumberOfValues );
  for ( int nCounter = 0; nCounter < m_lpParameters->nNumberOfMeasuring; ++nCounter )
  {
    for ( int nValue = 0; nValue < m_lpParameters->nNumberOfValues; ++ nValue )
    {      
      lpCount    = lpCounts;   lpCount += nValue;
      lpCalc     = m_lpValues; lpCalc  += nValue;
      lpCurrent  = lpData;     lpCurrent += (nCounter*m_lpParameters->nNumberOfValues)+nValue;
      if ( (!isnan(*lpCurrent))  && (*lpCurrent != m_lpParameters->fErrorValue)  )
      {
        ++(*lpCount);  
        *lpCalc += *lpCurrent;
      }
    }
  }

  for ( int nValue = 0; nValue < m_lpParameters->nNumberOfValues; ++ nValue )
  {
      lpCount  = lpCounts;   lpCount += nValue;
      lpCalc   = m_lpValues; lpCalc  += nValue;
      *lpCalc /= *lpCount;    
  }
  free( lpCounts );
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
float ADSensor::getValue(int nIndex )
//---------------------------------------------------------------------------------------
{
  if ( 0<= nIndex && nIndex < m_lpParameters->nNumberOfValues )
  {
    float* lpValue = m_lpValues;
    lpValue += nIndex;
    return ( isnan(*lpValue) ? m_lpParameters->fErrorValue : *lpValue);
  }
  return m_lpParameters->fErrorValue;
}
//---------------------------------------------------------------------------------------
long ADSensor::getMeasuringTime()
//---------------------------------------------------------------------------------------
{
  return m_lpParameters->lWarmingUp + ( m_lpParameters->nNumberOfMeasuring-1 ) * m_lpParameters->lWaitingTime;
}
//---------------------------------------------------------------------------------------
