#include <SoftwareSerial.h>

#ifndef SERVER_H
#define SERVER_H

#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "res/ajdos-setup.html.h"
#include "res/ajdos-setup.js.h"
#include "res/bootstrap.min.css.h"
#include "res/bootstrap.min.js.h"
#include "res/jquery-3.5.1.min.js.h"
#include "res/favico.ico.h"

#define     WS_NONE             0x00
#define     WS_PRESENT          0x01
#define     WS_READY            0x03

#define     WS_MODE_NOWIFI      0xff
#define     WS_MODE_SLEEP       0x00
#define     WS_MODE_CLIENT      0x01
#define     WS_MODE_SERVER      0x02


const int   MAX_CONNECT_TRIAL = 150;                 // cca 30 sec
const int   CONNECT_DELAY     = 500;                 // wait for n ms
const char* PROTOCOL          = "http://";
const char* HOSTNAME          = "AJDOS-SENSOR";
const char* HOSTDOMAIN        = "ajdos.go";
const byte  DNS_PORT          = 53;

IPAddress           g_APIP(192, 168, 1, 1);
CFLASHDATA*         g_lpFLASH;
DNSServer*          g_lpDNSServer;
ESP8266WebServer*   g_lpWEBServer;

char                g_CurrentHostPassword[64];

//=======================================================================================
void handleNotFound()
//---------------------------------------------------------------------------------------
// redirect to the main page if not in debug
//
{
#ifdef DEBUG
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += g_lpWEBServer->uri();
  message += "\nMethod: ";
  message += (g_lpWEBServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += g_lpWEBServer->args();
  message += "\n";

  for (uint8_t i = 0; i < g_lpWEBServer->args(); i++) {
    message += " " + g_lpWEBServer->argName(i) + ": " + g_lpWEBServer->arg(i) + "\n";
  }
  LOGLN( message.c_str() );
  g_lpWEBServer->send(404, "text/plain", message );
#else
  String strURL = PROTOCOL + String(HOSTDOMAIN);
  g_lpWEBServer->sendHeader("Location", strURL, true);
  g_lpWEBServer->send(301, "text/plain", "");  
#endif
}
//=======================================================================================
void handleRoot()
//---------------------------------------------------------------------------------------
{
    LOGLN("Root Handle");    
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "text/html", ajdos_html, ajdos_html_len ); 
}
//=======================================================================================
void handleJQuery()
//---------------------------------------------------------------------------------------
{
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "text/javascript", jquery_js, jquery_js_len ); 
}
//=======================================================================================
void handleBootstrapjs()
//---------------------------------------------------------------------------------------
{
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "text/javascript", bstrap_js, bstrap_js_len ); 
}
//=======================================================================================
void handleBootstrapcss()
//---------------------------------------------------------------------------------------
{
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "text/css", bstrap_css, bstrap_css_len ); 
}
//=======================================================================================
void handleAjdosjs()
//---------------------------------------------------------------------------------------
{
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "text/html", ajdos_js, ajdos_js_len ); 
}
//=======================================================================================
void favicon()
//---------------------------------------------------------------------------------------
{
    g_lpWEBServer->sendHeader("Content-Encoding","gzip", true);
    g_lpWEBServer->send( 200, "image/x-icon", ico, ico_len ); 
}
//=======================================================================================
void properties()
//---------------------------------------------------------------------------------------
{    
    String propData = "{";
    propData.concat( g_lpFLASH->getSensorJSON()   );
    propData.concat( "," );
    propData.concat( g_lpFLASH->getMeasuringJSON() );
    propData.concat( "," );    
    propData.concat( g_lpFLASH->getWifiJSON()     );
    propData.concat( "," );
    propData.concat( g_lpFLASH->getMqttJSON()     );
    propData.concat( "," );
    propData.concat( g_lpFLASH->getAdminJSON()     );
    propData.concat( "," );        
    propData.concat( g_lpFLASH->getTestJSON()     );
    propData.concat( "}" );       
    g_lpWEBServer->send( 200, "application/json", propData ); 
}
//=======================================================================================
void reboot()
//---------------------------------------------------------------------------------------
{
  APPDATA* lpAppData = g_lpFLASH->getApp();
  lpAppData->ModeAfterReset = APPMODE_SENSOR;
  g_lpFLASH->Write();
  g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
  delay(500);
  ESP.restart();
}
//=======================================================================================
void test()
//---------------------------------------------------------------------------------------
{
  APPDATA* lpAppData = g_lpFLASH->getApp();
  lpAppData->ModeAfterReset = APPMODE_TEST;
  g_lpFLASH->Write();
  g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
  delay(500);
  ESP.restart();
}
//=======================================================================================
void resetsensor()
//---------------------------------------------------------------------------------------
{
  APPDATA* lpAppData = g_lpFLASH->getApp();
  lpAppData->ModeAfterReset = APPMODE_RESET;
  g_lpFLASH->Write();
  g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
  delay(500);
  ESP.restart();
}
//=======================================================================================
void setsensor()
//---------------------------------------------------------------------------------------
{   
   LOGF("setsensor %d\n", g_lpWEBServer->args() );
    if ( g_lpWEBServer->args() >= 3 )
    {
      bool bChanged = false;      
      for (uint8_t i = 0; i < g_lpWEBServer->args(); i++)
      {
        if ( g_lpWEBServer->argName(i) == String("name") && ( strcmp( g_lpFLASH->getSensor()->Name, g_lpWEBServer->arg(i).c_str() ) != 0 ))
        {
            g_lpFLASH->getMqtt()->SendHello = true;
            strncpy( g_lpFLASH->getSensor()->Name, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("password") && ( strcmp( g_lpFLASH->getSensor()->Password, g_lpWEBServer->arg(i).c_str() ) != 0 ))
        {
            strncpy( g_lpFLASH->getSensor()->Password, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("owner") && ( strcmp( g_lpFLASH->getSensor()->OwnerId, g_lpWEBServer->arg(i).c_str() ) != 0 ))
        {
            g_lpFLASH->getMqtt()->SendHello = true;            
            strncpy( g_lpFLASH->getSensor()->OwnerId, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("station") && ( strcmp( g_lpFLASH->getSensor()->StationId, g_lpWEBServer->arg(i).c_str() ) != 0 ))
        {
            g_lpFLASH->getMqtt()->SendHello = true;            
            strncpy( g_lpFLASH->getSensor()->StationId, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }                
        if ( g_lpWEBServer->argName(i) == String("lat") && ( g_lpFLASH->getSensor()->Lat != g_lpWEBServer->arg(i).toFloat() ))
        {
            g_lpFLASH->getMqtt()->SendHello = true;
            g_lpFLASH->getSensor()->Lat = g_lpWEBServer->arg(i).toFloat();
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("long") && ( g_lpFLASH->getSensor()->Long != g_lpWEBServer->arg(i).toFloat() ))
        {
            g_lpFLASH->getMqtt()->SendHello = true;
            g_lpFLASH->getSensor()->Long = g_lpWEBServer->arg(i).toFloat();
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("alt") && (g_lpFLASH->getSensor()->Alt != g_lpWEBServer->arg(i).toFloat()) )
        {
            g_lpFLASH->getSensor()->Alt = g_lpWEBServer->arg(i).toFloat();
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("height") && (g_lpFLASH->getSensor()->Height != g_lpWEBServer->arg(i).toFloat()) )
        {
            g_lpFLASH->getSensor()->Height = g_lpWEBServer->arg(i).toFloat();
            bChanged = true;
        }
      }
      if ( bChanged )
      {
        LOGF( "%s",g_lpFLASH->getMqtt()->SendHello ? "Need to send hello message\n" : "" );
        g_lpFLASH->Write();
        g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
      }
      else
        g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
      return;
    }
    g_lpWEBServer->send( 301, "application/json", "{ \"code\": 301 }" );
}
//=======================================================================================
void setmeasuring()
//---------------------------------------------------------------------------------------
{  
  LOGF("setmeasuring %d\n", g_lpWEBServer->args() ); 
    if ( g_lpWEBServer->args() >= 2 )
    {
      
      bool   bChanged = false;
      for (uint8_t i = 0; i < g_lpWEBServer->args(); i++)
      {
        String Value( g_lpWEBServer->arg(i) );
        Value.replace("'","" );
        if ( g_lpWEBServer->argName(i) == String("cycle") && (g_lpFLASH->getApp()->SleepTime != Value.toInt()) )
        {
          g_lpFLASH->getApp()->SleepTime = Value.toInt();
          bChanged = true;          
        }
        if ( g_lpWEBServer->argName(i) == String("lag") && (g_lpFLASH->getApp()->LagTime != Value.toInt()) )
        {
          g_lpFLASH->getApp()->LagTime = Value.toInt();
          bChanged = true;
        }        
      }      
      if ( bChanged )
      {
        g_lpFLASH->Write();
        g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
      }
      else
        g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
      return;
    }
    g_lpWEBServer->send( 301, "application/json", "{ \"code\": 301 }" );
}
//=======================================================================================
void wifilist()
//---------------------------------------------------------------------------------------
{
  int n = WiFi.scanNetworks();
  if ( n == 0 )
  {
    g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
    return;
  }

  String propData = "{ \"ssid\":[ ";
  for (int i = 0; i < n; i++)
  {
    if ( i > 0 ) 
      propData.concat( ", " );
    propData.concat( "\"" );
    propData.concat( WiFi.SSID(i) );
    propData.concat( "\"" );
  }
  propData.concat( "] }" );   
  g_lpWEBServer->send( 200, "application/json", propData ); 
}
//=======================================================================================
void setwifi()
//---------------------------------------------------------------------------------------
{
  LOGF("setwifi %d\n", g_lpWEBServer->args() );
    if ( g_lpWEBServer->args() >= 2 )
    {
      bool bChanged = false;      
      for (uint8_t i = 0; i < g_lpWEBServer->args(); i++)
      {
        if ( g_lpWEBServer->argName(i) == String("ssid") )
        {
            strncpy( g_lpFLASH->getWifi()->Connect[0].SSID, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("pwd") )
        {
            g_lpFLASH->getApp()->TestResult = false;
            g_lpFLASH->getWifi()->Tested    = false;          
            strncpy( g_lpFLASH->getWifi()->Connect[0].Password, g_lpWEBServer->arg(i).c_str(), 64 );
            bChanged = true;
        }
      }
      if ( bChanged )
      {
        g_lpFLASH->Write();
        g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
      }
      else
        g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
      return;
    }
    g_lpWEBServer->send( 301, "application/json", "{ \"code\": 301 }" );
    
}
//=======================================================================================
void setmqtt()
//---------------------------------------------------------------------------------------
{
  LOGF("setmqtt %d\n", g_lpWEBServer->args() );
    if ( g_lpWEBServer->args() >= 4 )
    {
      bool bChanged = false;      
      for (uint8_t i = 0; i < g_lpWEBServer->args(); i++)
      {
        if ( g_lpWEBServer->argName(i) == String("ip") )
        {
            strncpy( g_lpFLASH->getMqtt()->IP, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("port") )
        {
            g_lpFLASH->getMqtt()->Port = g_lpWEBServer->arg(i).toInt();
            bChanged = true;
        }
        if ( g_lpWEBServer->argName(i) == String("user") )
        {
            strncpy( g_lpFLASH->getMqtt()->User, g_lpWEBServer->arg(i).c_str(), 32 );
            bChanged = true;
        }
        
        if ( g_lpWEBServer->argName(i) == String("pwd") )
        {
            strncpy( g_lpFLASH->getMqtt()->Password, g_lpWEBServer->arg(i).c_str(), 64 );
            bChanged = true;
        }
      }
      if ( bChanged )
      {
        g_lpFLASH->getApp()->TestResult = false;
        g_lpFLASH->getMqtt()->Tested    = false;
        g_lpFLASH->Write();
        g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
      }
      else
        g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
      return;
    }
    g_lpWEBServer->send( 301, "application/json", "{ \"code\": 301 }" );

}
//=======================================================================================
void setadmin()
//---------------------------------------------------------------------------------------
{
  LOGF("setadmin %d\n", g_lpWEBServer->args() );
    if ( g_lpWEBServer->args() >= 2 )
    {
      String oldPwd;
      String newPwd;
      
      bool bChanged = false;      
      for (uint8_t i = 0; i < g_lpWEBServer->args(); i++)
      {
        if ( g_lpWEBServer->argName(i) == String("pwdold") )
        {
            oldPwd = g_lpWEBServer->arg(i);
        }
        if ( g_lpWEBServer->argName(i) == String("pwdnew") )
        {
            newPwd = g_lpWEBServer->arg(i);
            bChanged = true;
        }
      }
      if ( bChanged )
      {
        String cmpPwd( g_CurrentHostPassword );
        if ( cmpPwd == oldPwd )
        {
          strncpy( g_lpFLASH->getSensor()->HostPassword, newPwd.c_str(), 64 );
          strncpy( g_CurrentHostPassword, newPwd.c_str(), 64 );
          g_lpFLASH->Write();
          g_lpWEBServer->send( 200, "application/json", "{ \"code\": 200 }" );
        }
        else
        {
          g_lpWEBServer->send( 403, "application/json", "{ \"code\": 403 }" );
          return;
        }
      }
      else
        g_lpWEBServer->send( 404, "application/json", "{ \"code\": 404 }" );
      return;
    }
    g_lpWEBServer->send( 301, "application/json", "{ \"code\": 301 }" );
}
//=======================================================================================




/****************************************************************************************/
class CWEBSERVER
/*--------------------------------------------------------------------------------------
 * 
 * HTTP requests:
 *          GET bootstrap.min.js  > content
 *          GET ajdos-setup.js    > content
 *          GET properties        > { "main" : { "name":"0003845", "lat":47.44209259, "long":19.2046411 }, "wifi" : { "ssid":"FincyCOM2", "pwd":"Kocsog0905" }, "send" : { "ip": "192.168.0.13", "port": 1883, "user":"nasquito", "pwd":"OpenHuboid8266" } }
 *          POST set              > 
 *          
 * 
  --------------------------------------------------------------------------------------*/
{
  protected:                  
      int          m_nStatus;
      int          m_nMode;
      
  public:
      
      CWEBSERVER( CFLASHDATA* lpFLASH );
      ~CWEBSERVER();

      bool    StartWEBServer();
      bool    run();
};
/****************************************************************************************/


//=======================================================================================
CWEBSERVER::CWEBSERVER(CFLASHDATA* lpFLASH )
//---------------------------------------------------------------------------------------
// Constructor
{
    g_lpFLASH     = lpFLASH;
    g_lpDNSServer = NULL;
    g_lpWEBServer = NULL;
    m_nMode       = WS_MODE_SLEEP;
    m_nStatus     = WL_IDLE_STATUS;
    
    
    if ( WiFi.status() == WL_NO_SHIELD )
    {
        m_nMode = WS_MODE_NOWIFI;
        LOGLN("WiFi shield not present");
    }      
}
//=======================================================================================
CWEBSERVER::~CWEBSERVER()
//---------------------------------------------------------------------------------------
{
  if ( g_lpWEBServer )
  {
    g_lpWEBServer->stop();
    g_lpWEBServer->close();
    delete g_lpWEBServer; g_lpWEBServer = NULL;
  }
  
  if ( g_lpDNSServer )
  {
    g_lpDNSServer->stop();
    delete g_lpDNSServer; g_lpDNSServer = NULL;
  }
}
//=======================================================================================



//=======================================================================================
bool CWEBSERVER::StartWEBServer()
//---------------------------------------------------------------------------------------
// Connect to WIFI as an Acces Point, create DNS and WEB server
//
{
    int  nCount     = 0;
    bool bConnected = false;

    switch (m_nMode)
    {
      case WS_MODE_NOWIFI:
           return false;
           
      case WS_MODE_SLEEP:

           g_lpDNSServer = new DNSServer();
           g_lpWEBServer = new ESP8266WebServer(80);

           strncpy( g_CurrentHostPassword, g_lpFLASH->getSensor()->HostPassword, 64 );

           WiFi.mode(WIFI_AP);
           WiFi.softAPConfig(g_APIP, g_APIP, IPAddress(255, 255, 255, 0));
           WiFi.softAP( HOSTNAME, g_CurrentHostPassword );

           g_lpDNSServer->setTTL(300);
           g_lpDNSServer->setErrorReplyCode(DNSReplyCode::ServerFailure);
           g_lpDNSServer->start(DNS_PORT, HOSTDOMAIN, g_APIP);

           g_lpWEBServer->onNotFound( handleNotFound );                    
           g_lpWEBServer->on("/",                    handleRoot         );
           g_lpWEBServer->on("/jquery-3.5.1.min.js", handleJQuery       );
           g_lpWEBServer->on("/bootstrap.min.js",    handleBootstrapjs  );
           g_lpWEBServer->on("/bootstrap.min.css",   handleBootstrapcss );
           g_lpWEBServer->on("/ajdos-setup.js",      handleAjdosjs      );          
           g_lpWEBServer->on("/favicon.ico",         favicon            );
           
           g_lpWEBServer->on("/properties",          properties );
           g_lpWEBServer->on("/reboot",              reboot     );
           g_lpWEBServer->on("/resetsensor",         resetsensor);
           g_lpWEBServer->on("/wifilist",            wifilist   );
           g_lpWEBServer->on("/test",                test       );
           
           g_lpWEBServer->on("/set-sensor",          setsensor  );           
           g_lpWEBServer->on("/set-measuring",       setmeasuring );                      
           g_lpWEBServer->on("/set-wifi",            setwifi    );
           g_lpWEBServer->on("/set-mqtt",            setmqtt    );
           g_lpWEBServer->on("/set-admin",           setadmin   );
           
           g_lpWEBServer->begin();

           m_nMode = WS_MODE_SERVER;
           LOGLN("DNS and WEB servers started.\n IPAddress: 192.168.1.1 \n");
           break;
           
    case WS_MODE_SERVER:
           break;
    }    
    return true;
}
//=======================================================================================



//=======================================================================================
bool CWEBSERVER::run()
//---------------------------------------------------------------------------------------
// loop cycle
{
  g_lpDNSServer->processNextRequest();
  g_lpWEBServer->handleClient();
  return true;
}
//=======================================================================================


#endif
