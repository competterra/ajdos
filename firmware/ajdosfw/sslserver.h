#ifndef SSLSERVER_H
#define SSLSERVER_H

// https://github.com/esp8266/Arduino/blob/2.7.4/libraries/DNSServer/examples/DNSServer/DNSServer.ino
// https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html
//
// HTTPS>>> https://www.onetransistor.eu/2019/04/https-server-on-esp8266-nodemcu.html
// openssl
// >genrsa -out ajdoskey.txt 1024
// >rsa -in ajdoskey.txt -out ajdoskey.txt
// >req -sha256 -new -nodes -key ajdoskey.txt -out ajdoscert.csr -subj '/C=HU/ST=Budapest/L=Budapest/O=Competterra/OU=IoT/CN=ajdos.go' -addext subjectAltName=DNS:ajdos.go
// >x509 -req -sha256 -days 3650 -in ajdoscert.csr -signkey ajdoskey.txt -out ajdoscert.txt
//
// Tanúsítványt újra kellene gyártani az -addext paraméterrel, mert azt a NAS nem tudta. Esetleg a Raspberryvel. :( ott sincs
// https://docs.joshuatz.com/cheatsheets/security/self-signed-ssl-certs/
//


#include    <ESP8266WiFi.h>
#include    <DNSServer.h>
#include    <ESP8266WebServer.h>
#include    <ESP8266WebServerSecure.h>

#include    "res/ajdos-setup.html.h"
#include    "res/ajdos-setup.js.h"
#include    "res/bootstrap.min.css.h"
#include    "res/bootstrap.min.js.h"
#include    "res/jquery-3.5.1.min.js.h"

#define     WS_NONE             0x00
#define     WS_PRESENT          0x01
#define     WS_READY            0x03

#define     WS_MODE_NOWIFI      0xff
#define     WS_MODE_SLEEP       0x00
#define     WS_MODE_CLIENT      0x01
#define     WS_MODE_SERVER      0x02


const int   MAX_CONNECT_TRIAL = 150;                 // cca 30 sec
const int   CONNECT_DELAY     = 500;                 // wait for n ms
const char* HOSTNAME          = "AJDOS-SENSOR";
const char* HOSTPASSWORD      = "12345678";
const char* HOSTDOMAIN        = "ajdos.go";
const byte  DNS_PORT          = 53;

IPAddress               g_APIP(192, 168, 1, 1);
extern CFLASHDATA*      g_lpFLASH;
DNSServer*              g_lpDNSServer;
ESP8266WebServer*       g_lpWEBServer;
ESP8266WebServerSecure* g_lpSSLServer;

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICSzCCAbQCCQDVvr4L6UjYQzANBgkqhkiG9w0BAQsFADBqMQswCQYDVQQGEwJI
VTERMA8GA1UECAwIQnVkYXBlc3QxETAPBgNVBAcMCEJ1ZGFwZXN0MRQwEgYDVQQK
DAtDb21wZXR0ZXJyYTEMMAoGA1UECwwDSW9UMREwDwYDVQQDDAhhamRvcy5nbzAe
Fw0yMTAyMTIyMTM4MDZaFw0zMTAyMTAyMTM4MDZaMGoxCzAJBgNVBAYTAkhVMREw
DwYDVQQIDAhCdWRhcGVzdDERMA8GA1UEBwwIQnVkYXBlc3QxFDASBgNVBAoMC0Nv
bXBldHRlcnJhMQwwCgYDVQQLDANJb1QxETAPBgNVBAMMCGFqZG9zLmdvMIGfMA0G
CSqGSIb3DQEBAQUAA4GNADCBiQKBgQD6nRkR3pjOGrthwbqXwKePCrjgQFx72HVB
X4Vcc48r62qFPnNOuHSofL4Fnfiv0Zh3WkuRk1qsyYGrKAVcrOgltVoHlYMThczb
ZblebA68VLp1+np4fCbT5SWBkWh9vj8EBZ90DytOyQGPZoRTs1YAznh4XoRWng2i
mfzTH2g23wIDAQABMA0GCSqGSIb3DQEBCwUAA4GBACpd62OTfWWlfwfIwR0EExZv
b8ooa0UJSPAd5Tx0KAgoAATjHSraq3u/IEG9+KH0wChpxqUveP04gWMU6zuJuwsN
v9VyaRE8MQc0joWG1/m4uFZTuouDjU/qqeICvLebQCALOVPioD/auJsTo8wW8yNG
r6of7ZUEVZjrUVVSUpIw
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQD6nRkR3pjOGrthwbqXwKePCrjgQFx72HVBX4Vcc48r62qFPnNO
uHSofL4Fnfiv0Zh3WkuRk1qsyYGrKAVcrOgltVoHlYMThczbZblebA68VLp1+np4
fCbT5SWBkWh9vj8EBZ90DytOyQGPZoRTs1YAznh4XoRWng2imfzTH2g23wIDAQAB
AoGABxnmePQmvkhRoO59XAlwdgQmwuvv8u5qPCT5wWng16y/AvGHEOb3H0MRm1Uj
k8/Lky4yz/gt7JF1JCVpKXwYw9Jp9vCkWQEFfuD95dEErB/7T/hjexciWfORp1rE
n93NpiQmTZlG0SXpk2o1zmba8pexEVGHFdLWtE+AtIXGMeECQQD9f9dD/vA+Ifi0
Px839IXn9k7dR7hTp0diluXoJ+4JIiMnZFMDVbTwMlf9Rgtgx3SftFgjYRlE0FHD
5U7lrYpvAkEA/RX4Q9QkvthBDtZclB8EkYOBLP1Vgph9OAfuuwnh8vB/BH5wGJkF
9sMtg9G0cuet3pSQXUU96RbVScBjNi8SkQJAObkBgHctp0g/BHQXHLuuPhqE7Lgn
Dc1aYOWxZkKpXiUK0i5zBKBgeg1ModxwPNJOCQucwjma8mNpKu0tIPl6OwJBALmW
NLm4hUGVXw+8TaMoo7b5Ee6lzuC3ZYuC9aMeehwmE15+QUEoZWEwet3hF0vzbGJ9
2ucqcV3NqpYsiBQhrKECQQDb4FWgqnBtJB+Y5evhqx6JMEeZsWsoFvTJZ+Gm7qcv
cUpbXUz67UIioerLqqpe23nsbUsfcHf2fSda0v2C7Mq5
-----END RSA PRIVATE KEY-----
)EOF";


//=======================================================================================
void handleRoot()
//---------------------------------------------------------------------------------------
{
    LOGLN("Root Handle");    
    g_lpSSLServer->sendHeader("Content-Encoding","gzip", true);
    g_lpSSLServer->send( 200, "text/html", ajdos_html, ajdos_html_len ); 
}
//=======================================================================================
void handleJQuery()
//---------------------------------------------------------------------------------------
{
    g_lpSSLServer->sendHeader("Content-Encoding","gzip", true);
    g_lpSSLServer->send( 200, "text/javascript", jquery_js, jquery_js_len ); 
}
//=======================================================================================
void handleBootstrapjs()
//---------------------------------------------------------------------------------------
{
    g_lpSSLServer->sendHeader("Content-Encoding","gzip", true);
    g_lpSSLServer->send( 200, "text/javascript", bstrap_js, bstrap_js_len ); 
}
//=======================================================================================
void handleBootstrapcss()
//---------------------------------------------------------------------------------------
{
    g_lpSSLServer->sendHeader("Content-Encoding","gzip", true);
    g_lpSSLServer->send( 200, "text/css", bstrap_css, bstrap_css_len ); 
}
//=======================================================================================
void handleAjdosjs()
//---------------------------------------------------------------------------------------
{
    g_lpSSLServer->sendHeader("Content-Encoding","gzip", true);
    g_lpSSLServer->send( 200, "text/html", ajdos_js, ajdos_js_len ); 
}
//=======================================================================================
void handleNotFound()
//---------------------------------------------------------------------------------------
{
  #ifdef DEBUG
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += g_lpSSLServer->uri();
    message += "\nMethod: ";
    message += (g_lpSSLServer->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += g_lpSSLServer->args();
    message += "\n";
  
    for (uint8_t i = 0; i < g_lpSSLServer->args(); i++) {
      message += " " + g_lpSSLServer->argName(i) + ": " + g_lpSSLServer->arg(i) + "\n";
    }
    LOGLN( message.c_str() );
    g_lpSSLServer->send(404, "text/plain", message );
  #else
    String strURL = "https://"+ String(HOSTDOMAIN);
    g_lpSSLServer->sendHeader("Location", strURL, true);
    g_lpSSLServer->send(301, "text/plain", "");
  #endif
}
//=======================================================================================
void secureRedirect()
//---------------------------------------------------------------------------------------
{
  String strURL = "https://"+ String(HOSTDOMAIN);
  g_lpWEBServer->sendHeader("Location", strURL, true);
  g_lpWEBServer->send(301, "text/plain", "");
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
      int                     m_nStatus;
      int                     m_nMode;
      char                    m_APPassword[64];
      
  public:
      
      CWEBSERVER();
      ~CWEBSERVER();

      bool    StartHTTPServer();
      bool    run();
};
/****************************************************************************************/


//=======================================================================================
CWEBSERVER::CWEBSERVER()
//---------------------------------------------------------------------------------------
// Constructor
{
    g_lpDNSServer = NULL;
    g_lpWEBServer = NULL;
    m_nMode       = WS_MODE_SLEEP;
    m_nStatus     = WL_IDLE_STATUS;
    memset( &m_APPassword, 0, sizeof(m_APPassword) );
    strcpy( m_APPassword, HOSTPASSWORD );    
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

  if ( g_lpWEBServer )
  {
    g_lpSSLServer->stop();
    g_lpSSLServer->close();
    delete g_lpSSLServer; g_lpSSLServer = NULL;
  }
  
  if ( g_lpDNSServer )
  {
    g_lpDNSServer->stop();
    delete g_lpDNSServer; g_lpDNSServer = NULL;
  }
}
//=======================================================================================




//=======================================================================================
bool CWEBSERVER::StartHTTPServer()
//---------------------------------------------------------------------------------------
// try to connect to WIFI. Exit if not connected after more then MAX_CONNCT_TRIAL trial.
//
{
    int nCount      = 0;
    bool bConnected = false;

    switch (m_nMode)
    {
      case WS_MODE_NOWIFI:
           return false;
           
      case WS_MODE_SLEEP:
           g_lpDNSServer = new DNSServer();
           g_lpSSLServer = new ESP8266WebServerSecure(443);
           g_lpWEBServer = new ESP8266WebServer(80);

           WiFi.mode(WIFI_AP);
           WiFi.softAPConfig(g_APIP, g_APIP, IPAddress(255, 255, 255, 0));
           WiFi.softAP( HOSTNAME, (( g_lpFLASH->isValid() && g_lpFLASH->getWiFi()->HostPassword[0] != '\0' ) ? g_lpFLASH->getWiFi()->HostPassword : m_APPassword ) );

           g_lpDNSServer->setTTL(300);
           g_lpDNSServer->setErrorReplyCode(DNSReplyCode::ServerFailure);
           g_lpDNSServer->start(DNS_PORT, HOSTDOMAIN, g_APIP);         
           
           g_lpWEBServer->on("/", secureRedirect );
           g_lpWEBServer->begin();

           g_lpSSLServer->onNotFound( handleNotFound );
           g_lpSSLServer->getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
           g_lpSSLServer->on("/", handleRoot);
           g_lpSSLServer->on("/jquery-3.5.1.min.js", handleJQuery );
           g_lpSSLServer->on("/bootstrap.min.js", handleBootstrapjs );
           g_lpSSLServer->on("/bootstrap.min.css", handleBootstrapcss );
           g_lpSSLServer->on("/ajdos-setup.js", handleAjdosjs );
           g_lpSSLServer->begin();

           m_nMode = WS_MODE_SERVER;
           LOGLN("DNS and WEB servers started.");
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
//
{
  g_lpDNSServer->processNextRequest();
  g_lpWEBServer->handleClient();
  g_lpSSLServer->handleClient();
  return true;
}
//=======================================================================================


#endif
