#ifndef APP_H
#define APP_H

#define   APPMODE_SETUP         0
#define   APPMODE_TEST          1
#define   APPMODE_SENSOR        2
#define   APPMODE_RESET         3

#include "flash.h"
#include "webserver.h"
#include "webclient.h"
#include "ajdos.h"

/****************************************************************************************/
class CAPP
/*--------------------------------------------------------------------------------------*/
{
  protected:
      int           m_nAppMode;
      CFLASHDATA*   m_lpFlash;
      CAJDOS*       m_lpSensor;      
      CWEBSERVER*   m_lpWebServer;
      CWEBCLIENT*   m_lpWebClient;
      
  public:
      CAPP();
      ~CAPP();

      void  freeAll();
      int   detectAppMode();
      void  setResetMode(int newAppMode, bool bResetNow = false );
      void  setMode( int newMode ) { m_nAppMode = newMode; m_lpFlash->getApp()->CurrentMode = newMode; }
      int   getMode()  { return m_nAppMode; }
      
      void  setWebServerMode();
      void  setTestMode();
      void  setSensorMode();
      void  setSensorResetMode();
      
      void  run();
      void  reset();
      void  deepSleep();
};
/****************************************************************************************/


//=======================================================================================
CAPP::CAPP()
//---------------------------------------------------------------------------------------
{
  m_lpSensor    = NULL;
  m_lpWebServer = NULL;
  m_lpWebClient = NULL;
  m_lpFlash     = new CFLASHDATA();
  m_lpFlash->Read();
  detectAppMode();
}
//=======================================================================================
CAPP::~CAPP()
//---------------------------------------------------------------------------------------
{
  freeAll();
}
//=======================================================================================
void CAPP::freeAll()
//---------------------------------------------------------------------------------------
{
  if ( m_lpFlash )     delete m_lpFlash;
  if ( m_lpSensor )    delete( m_lpSensor );    m_lpSensor    = NULL;
  if ( m_lpWebServer ) delete( m_lpWebServer ); m_lpWebServer = NULL;
  if ( m_lpWebClient ) delete( m_lpWebClient ); m_lpWebClient = NULL;
}
//=======================================================================================


//=======================================================================================
int CAPP::detectAppMode()
//---------------------------------------------------------------------------------------
{
  APPDATA* lpAppData = m_lpFlash->getApp();
  
  if ( lpAppData->ModeAfterReset == APPMODE_RESET ) 
  {
    m_nAppMode             = APPMODE_RESET;
    lpAppData->CurrentMode = APPMODE_RESET; 
    return APPMODE_RESET;
  }
  
  if ( lpAppData->Inicialized && lpAppData->ModeAfterReset == APPMODE_TEST )
  {
    m_nAppMode             = APPMODE_TEST;
    lpAppData->CurrentMode = APPMODE_TEST; 
    return APPMODE_TEST;
  }
      
  if ( (! lpAppData->Inicialized ) || lpAppData->ModeAfterReset == APPMODE_SETUP || (! lpAppData->TestResult) )
  {
    m_nAppMode             = APPMODE_SETUP;
    lpAppData->CurrentMode = APPMODE_SETUP; 
    return APPMODE_SETUP;
  }
  
  m_nAppMode             = APPMODE_SENSOR;
  lpAppData->CurrentMode = APPMODE_SENSOR; 
  return APPMODE_SENSOR;
}
//=======================================================================================
void CAPP::setResetMode( int newAppMode, bool bResetNow )
//---------------------------------------------------------------------------------------
{
  APPDATA* lpAppData = m_lpFlash->getApp();
  lpAppData->ModeAfterReset = newAppMode;
  if ( bResetNow )
    reset();
}
//=======================================================================================



//=======================================================================================
void CAPP::reset()
//---------------------------------------------------------------------------------------
{
  m_lpFlash->Write();
  freeAll();
  LOGLN("WEMOS RESET");
  ESP.restart(); 
}
//=======================================================================================
void CAPP::deepSleep()
//---------------------------------------------------------------------------------------
// lag is a random piece of delay added to the sleep time. Max value is set in the config
// web application.
{
  unsigned long nextLag = random( 0, m_lpFlash->getApp()->LagTime )*1000000; 
  unsigned long uSleep  = m_lpFlash->getApp()->SleepTime;
  uSleep = uSleep*60*1000000 + nextLag;
  
  m_lpFlash->getApp()->LastSleepingTime = uSleep;
  m_lpFlash->Write();
  freeAll();
  LOGF("WEMOS DEEP SLEEP (%d usec)\n", uSleep );
  ESP.deepSleep( uSleep );
}
//=======================================================================================



//=======================================================================================
void CAPP::setWebServerMode()
//---------------------------------------------------------------------------------------
{
  if ( m_lpWebServer = new CWEBSERVER(m_lpFlash) )
  {
    m_lpWebServer->StartWEBServer();
  }
  
}
//=======================================================================================
void CAPP::setTestMode()
//---------------------------------------------------------------------------------------
{
  if ( m_lpFlash->getApp()->Inicialized )
  {
    if ( m_lpWebClient == NULL )
       m_lpWebClient = new CWEBCLIENT( m_lpFlash );
  }
  else
      setWebServerMode();
}
//=======================================================================================
void CAPP::setSensorMode()
//---------------------------------------------------------------------------------------
{
  if ( m_lpSensor = new CAJDOS( m_lpFlash ) )
  {
    if ( m_lpWebClient == NULL )
         m_lpWebClient = new CWEBCLIENT( m_lpFlash );
    m_lpSensor->initSensors( m_lpWebClient );  
  }  
}
//=======================================================================================
void CAPP::setSensorResetMode()
//---------------------------------------------------------------------------------------
{
   if ( m_lpWebClient == NULL )
        m_lpWebClient = new CWEBCLIENT( m_lpFlash );
}
//=======================================================================================


//=======================================================================================
void CAPP::run()
//---------------------------------------------------------------------------------------
{
  switch ( m_nAppMode )
  {
    case APPMODE_SETUP:
      if ( m_lpWebServer ) m_lpWebServer->run();
      break;
    case APPMODE_TEST:
      if ( m_lpWebClient ) m_lpWebClient->run();
      break;
    case APPMODE_RESET:
      {
        CNTPTIMER*  m_lpTimer = new CNTPTIMER( m_lpFlash );
        time_t      calcTime  = m_lpTimer->getTime( false );
        m_lpWebClient->sendMQTTBye( m_lpTimer->getGMTTime( calcTime ) );
        m_lpFlash->Clear();
        reset();        
      }
      break;
    default:
      if ( m_lpSensor ) m_lpSensor->run();
      deepSleep();
      break;            
  }
}
//=======================================================================================

#endif
