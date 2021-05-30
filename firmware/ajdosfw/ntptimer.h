#ifndef NTPTIMER_H
#define NTPTIMER_H

#define MAX_NTPTRIAL 20
#include <NTPClient.h>
#include <WiFiUdp.h>

/****************************************************************************************/
class CNTPTIMER
/*--------------------------------------------------------------------------------------*/
{
  protected:
    CFLASHDATA*     m_lpFlash;
    unsigned long   m_ItemCount;
    bool            m_NTPSuccess;
    time_t          m_CurrentTime;
    
  public:
    CNTPTIMER(CFLASHDATA* lpFlash);
    ~CNTPTIMER();
    
  time_t  getTime( bool bConnected );
  char*   getGMTTime( time_t timeStamp );
};
/****************************************************************************************/


//=======================================================================================
CNTPTIMER::CNTPTIMER(CFLASHDATA* lpFlash)
//---------------------------------------------------------------------------------------
{
  m_lpFlash = lpFlash;  
}
//=======================================================================================
CNTPTIMER::~CNTPTIMER()
//---------------------------------------------------------------------------------------
{
  
}
//=======================================================================================


//=======================================================================================
time_t CNTPTIMER::getTime( bool bConnected )
//---------------------------------------------------------------------------------------
// Return the current NTP or calculataed time
{
    int nTryNTP = 0;
    m_NTPSuccess = false;
    if ( bConnected )
    {
      LOG("NTP server ");
      WiFiUDP   ntpUDP;
      NTPClient timeClient( ntpUDP, "europe.pool.ntp.org", 0 );
      timeClient.begin();
      delay(500);
      while( (!timeClient.update()) && (++nTryNTP <= MAX_NTPTRIAL) )
      {
        timeClient.forceUpdate();
        LOG("-");
      }      
      if ( nTryNTP <= MAX_NTPTRIAL )
      {
        LOGLN(" OK");
        m_CurrentTime = timeClient.getEpochTime();
        m_NTPSuccess  = true;
        m_lpFlash->getApp()->LastNTPTime     = m_CurrentTime;
        m_lpFlash->getApp()->NTPEllapsedTime = 0;  
        return m_CurrentTime;
      }
    }
    LOGLN(" NO NTP FOUND");
    m_CurrentTime  = (millis() / 1000);
    m_lpFlash->getApp()->NTPEllapsedTime += m_lpFlash->getApp()->LastSleepingTime/1000000 + m_CurrentTime;
    m_CurrentTime += m_lpFlash->getApp()->LastNTPTime + m_lpFlash->getApp()->NTPEllapsedTime;
    LOGF("Calculated time is %d\r\n", m_CurrentTime );
    return m_CurrentTime;    
}
//=======================================================================================


//=======================================================================================
char* CNTPTIMER::getGMTTime(time_t timeStamp )
//---------------------------------------------------------------------------------------
{
  struct       tm ts;
  static char  buf[32];
  
  ts = *localtime( &timeStamp );  
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S GMT", &ts);
  LOGF("The current NTP time is: %s\r\n", buf );
  return  (char*) &buf[0];      
}
//=======================================================================================


#endif
