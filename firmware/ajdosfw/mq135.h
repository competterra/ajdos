/*
 * 
 * AJDOS GAS sensor class v1.0
 * 
 * 
 * Competterra Ltd. Hungary 2020. by Fincy GO.
 * (C) MIT licence
 * 
 * dependencies: 
 *    
 *    hardware: use an analog multiplexer for measuring more analog voltage level
  *   CD4066                                                                            
 *      VDD connected                       to CD4066  1 PIN = Signal  A In  
 *      GAS Sensor + connected              to CD4066  2 PIN = Signal  A Out 
 *      GAS Sensor Analog Out connected     to CD4066 11 PIN = Signal  D In 
 *      CD4066 12 PIN = Control D connected to CD4066 13 PIN = Control A      
 *      Wemos D8 connected                  to CD4066 13 PIN = Control A    
 *      Wemos A0 analog input connected     to CD4066 10 PIN = Signal  D Out       
 *    
 *    HIGH level on D8 turns on A and B switch in CD4066, it means the GAS sensor power is on and GAS sensor analog output connected to the wemos A0 analog input pin.
 */


//=======================================================================================
class CMQ135 : public ADSensor
//---------------------------------------------------------------------------------------
{
protected:  
   SensorParameters m_Params;
   
public:   
   CMQ135();
   ~CMQ135();
   
   void             init();        
   bool             powerOn();
   void             powerOff();
   float            measuring( int nIndex );
   void             save();
};
//=======================================================================================


//---------------------------------------------------------------------------------------
CMQ135::CMQ135():ADSensor()
//---------------------------------------------------------------------------------------
{
}
//---------------------------------------------------------------------------------------
CMQ135::~CMQ135()
{
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CMQ135::init()
//---------------------------------------------------------------------------------------
{
  setDefaults( &m_Params );
  
  //todo: set min/max temperature and humidity
  
  m_Params.lWarmingUp            = 30000;       // 30 sec 
  m_Params.lWaitingTime          = 2000;        // 2 sec
  m_Params.nNumberOfMeasuring    =  5;
  m_Params.nNumberOfValues       =  1;          // input voltage
  m_Params.anRangeTemperature[0] = -10;
  m_Params.anRangeTemperature[1] =  45;
  pinMode( GAS_SWITCH,OUTPUT );
  pinMode( GAS_DATA, INPUT   );  
  digitalWrite( GAS_SWITCH, LOW  );
  ADSensor::init( &m_Params );
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
bool CMQ135::powerOn()
//---------------------------------------------------------------------------------------
// set 4051 address to 0
// 
{  
  digitalWrite( GAS_SWITCH, HIGH );
  return true;
}
//---------------------------------------------------------------------------------------
void CMQ135::powerOff()
{
  // todo: set all adrres HIGH and connect input 7 to GND on the IC
  digitalWrite( GAS_SWITCH, LOW  );
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
float CMQ135::measuring( int nIndex )
//---------------------------------------------------------------------------------------
{   
   return (float) 1. * analogRead(GAS_DATA);
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
void CMQ135::save()
//---------------------------------------------------------------------------------------
{
  
}
