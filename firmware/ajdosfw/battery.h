/*
 * 
 * AJDOS Battery sensor class v1.0
 * 
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 * dependencies: 
 *    not set ADC_MODE( ADC_VCC ); if measure external battery voltage
 *    resistor value 
 *    
 * hardware   
 *   CD4066                                                        C                    A  B  D 
 *      Battery connected                to CD4066 8 PIN = Signal  C In  (alternatives: 1, 4,11 PIN )
 *      Wemos A0 analog input connected  to CD4066 9 PIN = Signal  C Out (alternatives: 2, 3,10 PIN )
 *      Wemos D3 digital out connected   to CD4066 6 PIN = Control C     (alternatives:13, 5,12 PIN )
 *      
 *   A0 <----- R ------ Battery + 4.5V  
 *   R2  0     3,2V          A0 Value
 *   R2  120k  4.4V
 *   R2  130k  4.5V          1024  100%
 *   R2  147k  4.67V          987  100% 
 *   R2  150k  4.7V           980  100%
 *   R2  220k  5.4V           853  100% 
 *   R2  470k  7.9V           583  100%
 *   R2  500k  8,2V           561  100%
 *   
 *   
 */

 #define BAT_FULL 980       // R2 150k

//=======================================================================================
class CBattery : public ADSensor
//---------------------------------------------------------------------------------------
{
protected:  
   SensorParameters m_Params;
   
public:   
   CBattery();
   ~CBattery();
   
   void             init();        
   bool             powerOn();
   void             powerOff();
   float            measuring( int nIndex );
   void             save();
};
//=======================================================================================


//---------------------------------------------------------------------------------------
CBattery::CBattery():ADSensor()
//---------------------------------------------------------------------------------------
{
}
//---------------------------------------------------------------------------------------
CBattery::~CBattery()
{
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CBattery::init()
//---------------------------------------------------------------------------------------
{
  setDefaults( &m_Params );
  
  m_Params.lWarmingUp            = 1000;        // 1 sec 
  m_Params.lWaitingTime          = 1000;        // 1 sec
  m_Params.nNumberOfMeasuring    =  5;
  m_Params.nNumberOfValues       =  1;          // input voltage

  pinMode( BATTERY_SWITCH,      OUTPUT );
  digitalWrite( BATTERY_SWITCH, LOW  );
  ADSensor::init( &m_Params );
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
bool CBattery::powerOn()
//---------------------------------------------------------------------------------------
// set 4051 address to 1
// 
{  
  digitalWrite( BATTERY_SWITCH, HIGH  );
  return true;
}
//---------------------------------------------------------------------------------------
void CBattery::powerOff()
{
  digitalWrite( BATTERY_SWITCH, LOW  );
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
float CBattery::measuring( int nIndex )
//---------------------------------------------------------------------------------------
{
   int nLevel = analogRead(BATTERY_DATA);   
   return ((float) 1. * ( nLevel > BAT_FULL ? BAT_FULL : nLevel )) / BAT_FULL * 100.;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CBattery::save()
//---------------------------------------------------------------------------------------
{
  
}
