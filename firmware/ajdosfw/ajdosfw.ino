

/*** WARNINGS *************************************************************************
 *  
 *  Set BAT_FULL constant in battery.h it depends on the used R2 resistor (Now 150kOhm)
 *  Comment the #define DEBUG below in the live version
 *  
 *  TODO: Currently not topic here
 *  
 **************************************************************************************/

#define  DEBUG
#include "debug.h"
#include "time.h"
#include "application.h"

CAPP*    glpApp = NULL;            

   
//**************************************************************************************
void setup()
//======================================================================================
{
  startSerial();
  glpApp = new CAPP();

  pinMode( APP_SETUP_SWITCH, INPUT );
  if ( digitalRead(APP_SETUP_SWITCH) == LOW )
  {
    glpApp->detectAppMode();
    if (glpApp->getMode() == APPMODE_SENSOR )
      glpApp->setMode( APPMODE_SETUP );
  }
  switch ( glpApp->getMode() )
  {
    case APPMODE_SETUP:        
        LOGLN("*** RUN IN SETUP MODE ***");
        glpApp->setResetMode( APPMODE_SETUP );
        glpApp->setWebServerMode();
        break;
        
    case APPMODE_TEST:
        LOGLN("*** RUN IN TEST MODE ***");
        glpApp->setTestMode();
        glpApp->run();
        glpApp->setWebServerMode();
        glpApp->setResetMode( APPMODE_SETUP );
        glpApp->reset();
        break;

    case APPMODE_RESET:
        LOGLN("*** RUN IN SENSOR RESET ***");        
        glpApp->setSensorResetMode();
        break;        
        
    default:
        LOGLN("*** RUN IN SENSOR MODE ***");        
        glpApp->setSensorMode();
        glpApp->run();
        glpApp->setResetMode( APPMODE_SENSOR );
        glpApp->deepSleep();
        break;        
  }
}
//======================================================================================


//======================================================================================
void startSerial()
//--------------------------------------------------------------------------------------  
{
  Serial.begin(9600);
  while (!Serial ) delay(50);  
  LOGLN("\n\n\n");
  LOGLN("START WEMOS D1");  
}
//======================================================================================


//======================================================================================
void loop() 
//--------------------------------------------------------------------------------------
{
  glpApp->run();  
}
//======================================================================================
