#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "ntptimer.h"

/****************************************************************************************/
class CDATAHANDLER
/*--------------------------------------------------------------------------------------*/
{
  protected:
    CFLASHDATA*     m_lpFlash;
    CWEBCLIENT*     m_lpWebClient;
    CNTPTIMER*      m_lpTimer;
    unsigned long   m_ItemCount;
    
  public:
    CDATAHANDLER(CFLASHDATA* lpFlash );
    ~CDATAHANDLER();

  void          initHandler(CWEBCLIENT* lpWebClient);
  MEASURE*      getCurrentMeasure();    
  void          addNewMeasures();
  void          setTemperature(float Temperature, float Humidity);
  void          setGas(float Gas);
  void          setBattery(float Battery);
  void          setPM(float pm125, float pm10);
  unsigned int  getItemCount() { return m_lpFlash->getApp()->MeasureItemCount; }
  bool          sendMeasures(bool bSendMQTT = true );
  void          rollUp();

  void          FIFODebug();
};
/****************************************************************************************/


//=======================================================================================
CDATAHANDLER::CDATAHANDLER(CFLASHDATA* lpFlash )
//---------------------------------------------------------------------------------------
{
  m_lpFlash     = lpFlash;
  m_ItemCount   = MAX_SAVED_MEASURES;
  m_lpTimer     = new CNTPTIMER( m_lpFlash );
}
//=======================================================================================
CDATAHANDLER::~CDATAHANDLER()
//---------------------------------------------------------------------------------------
{
  delete m_lpTimer;
  m_lpTimer = NULL;
}
//=======================================================================================
void CDATAHANDLER::initHandler(CWEBCLIENT* lpWebClient)
//---------------------------------------------------------------------------------------
{
  m_lpWebClient = lpWebClient;
}
//=======================================================================================

//=======================================================================================
void CDATAHANDLER::rollUp()
//---------------------------------------------------------------------------------------
{
    int index = 0;
    MEASURE* lpFIFO    = m_lpFlash->getMeasures();
    MEASURE* lpCurrent = lpFIFO;
    MEASURE* lpNext    = lpCurrent + 1;
    for ( index = 0; index < MAX_SAVED_MEASURES-1; ++index )
    {
      lpNext    = lpCurrent + 1;
      memcpy( lpCurrent, lpNext, sizeof(MEASURE) );
      lpCurrent = lpNext;
    }
    memset( lpNext, 0,  sizeof(MEASURE) ); 
}
//=======================================================================================


//=======================================================================================
void CDATAHANDLER::addNewMeasures()
//---------------------------------------------------------------------------------------
// add new element at the end of the list. Roll up the list if no more available items
//
{
  if ( m_lpFlash->getApp()->MeasureItemCount == MAX_SAVED_MEASURES )
  {
    rollUp();
  }
  else
    ++m_lpFlash->getApp()->MeasureItemCount;
  MEASURE* lpNewItem = getCurrentMeasure();  
  lpNewItem->Size = sizeof( MEASURE );
}
//=======================================================================================
MEASURE* CDATAHANDLER::getCurrentMeasure()
//---------------------------------------------------------------------------------------
// return the last element from the list
{
  MEASURE* lpFIFO = m_lpFlash->getMeasures();
  lpFIFO += (m_lpFlash->getApp()->MeasureItemCount-1);
  return lpFIFO;
}
//=======================================================================================


//=======================================================================================
void CDATAHANDLER::setTemperature(float Temperature, float Humidity)
//---------------------------------------------------------------------------------------
{  
    MEASURE* lpItem = getCurrentMeasure();
    lpItem->Temperature = Temperature;
    lpItem->Humidity    = Humidity;
    if (lpItem->Temperature != DEF_ERRORVALUE)
       lpItem->Mask |= 1; 
}
//=======================================================================================
void CDATAHANDLER::setBattery(float Battery)
//---------------------------------------------------------------------------------------
{
    MEASURE* lpItem = getCurrentMeasure();
    lpItem->Battery = Battery;
    if (lpItem->Battery != DEF_ERRORVALUE)
       lpItem->Mask |= 4;            
}
//=======================================================================================
void CDATAHANDLER::setGas(float Gas)
//---------------------------------------------------------------------------------------
{
    MEASURE* lpItem = getCurrentMeasure();
    lpItem->GasValue = Gas;
    if (lpItem->GasValue != DEF_ERRORVALUE)
       lpItem->Mask |= 8;            
}
//=======================================================================================
void CDATAHANDLER::setPM(float pm25, float pm10)
//---------------------------------------------------------------------------------------
{
    MEASURE* lpItem = getCurrentMeasure();
    lpItem->Pm25 = pm25;
    lpItem->Pm10 = pm10;
    if (lpItem->Pm10 != DEF_ERRORVALUE)
       lpItem->Mask |= 2;            
}
//=======================================================================================


//=======================================================================================
bool CDATAHANDLER::sendMeasures( bool bSendMQTT )
//---------------------------------------------------------------------------------------
{
    MEASURE* lpItem = getCurrentMeasure();
    if (bSendMQTT && m_lpWebClient && m_lpWebClient->beginSendData() )
    {
        lpItem->Timestamp = m_lpTimer->getTime( true );
        LOGF( "%s\n",m_lpFlash->getMqtt()->SendHello ? "Sending hello message" : "No extra messages needed" );
        if ( m_lpFlash->getMqtt()->SendHello )
        {
            if ( m_lpFlash->getMqtt()->SendBye )
                 m_lpWebClient->sendMQTTBye( m_lpTimer->getGMTTime( lpItem->Timestamp ) );
            m_lpWebClient->sendMQTTHello( m_lpTimer->getGMTTime( lpItem->Timestamp ) );                 
            m_lpFlash->getMqtt()->SendHello = false;
            m_lpFlash->getMqtt()->SendBye   = true;
        }
        while ( m_lpFlash->getApp()->MeasureItemCount > 0 )
        {
          MEASURE* lpItem  = m_lpFlash->getMeasures();
          LOGF("Send MQTT Data > i:%0.0f, %0.2f, %0.2f, %s\n", lpItem->Battery, lpItem->Temperature, lpItem->Humidity, m_lpTimer->getGMTTime( lpItem->Timestamp ) );
          m_lpWebClient->sendMQTTData( lpItem, m_lpTimer->getGMTTime( lpItem->Timestamp ) );         
          rollUp();
          --m_lpFlash->getApp()->MeasureItemCount;
        }
        m_lpWebClient->endSendData();
    }
    else
    {
        lpItem->Timestamp = m_lpTimer->getTime( false );
        lpItem->Sended    = false;
    }
    return false;    
}
//=======================================================================================



void CDATAHANDLER::FIFODebug()
{
  MEASURE* lpItem = m_lpFlash->getMeasures();
  
  for ( int index = 0; index < MAX_SAVED_MEASURES; ++index )
  {
    LOGF("%i. s:%d, b:%0.2f, t:%0.2f, h:%0.2f, T:%d\n", index, lpItem->Size, lpItem->Battery, lpItem->Temperature, lpItem->Humidity, lpItem->Timestamp );
    ++lpItem;
  }
  
}

#endif
