#include <WiFi.h>
//#include <FS.h>
//https://github.com/me-no-dev
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
  
const char* ssid = "yourssid";
const char* password = "pass"; 

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ESP8266 Arduino</title>
<style>
"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
#container
{width: 500px;
margin-left: auto;
margin-right: auto;}
#pol1
{width: 400px;
background-color: yellow;
color: red;
text-align: left;
padding: 50px;}

</style>
<script>
var websock;
function start() {
websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  
  websock.onmessage = function(evt) {
    console.log(evt);
  var str = evt.data;
    var res = str.split("@&");
     console.log(res[0]);
   if (res [0]=== 'start')
   {
    console.log('rozpoznal');
    document.getElementById("txt1").value = res[1];
    document.getElementById("txt2").value = res[2];
    document.getElementById("txt3").value = res[3];
    document.getElementById("txt4").value = res[4];
   }
    var e = document.getElementById('led1status');
    var f = document.getElementById('led2status');
    var g = document.getElementById('led3status');
    var h = document.getElementById('led4status');
 
    if (evt.data === 'led1on') {
      e.style.color = 'red';
    }
    else if (evt.data === 'led1off') {
      e.style.color = 'black';
    }
    else if (evt.data === 'led2on') {
      f.style.color = 'red';
    }
    else if (evt.data === 'led2off') {
      f.style.color = 'black';
    }
    else if (evt.data === 'led3on') {
      g.style.color = 'red';
    }
    else if (evt.data === 'led3off') {
      g.style.color = 'black';
    }
    else if (evt.data === 'led4on') {
      h.style.color = 'red';
    }
    else if (evt.data === 'led4off') {
      h.style.color = 'black';
    }

    else {
      console.log('unknown event');
    }

  };
}
function buttonclick(e) {
  websock.send(e.id);
}
</script>
</head>
<body onload="javascript:start();">
<div id="container">
<div id="pol1">
<h1>ESP8266 Arduino</h1>
<div id="txta1"><b>T1=</b> <INPUT TYPE="text" id="txt1" NAME="text1" VALUE="N/A   " SIZE=5>&deg F</div>
<div id="txta2"><b>T2=</b> <INPUT TYPE="text" id="txt2" NAME="text2" VALUE="N/A   " SIZE=5>&deg F</div>
<div id="txta3"><b>T3=</b> <INPUT TYPE="text" id="txt3" NAME="text3" VALUE="N/A   " SIZE=5>&deg F</div>
<div id="txta4"><b>T4=</b> <INPUT TYPE="text" id="txt4" NAME="text4" VALUE="N/A   " SIZE=5>&deg C</div>
<div id="tex5">............................... </div>
<div id="led1status"><b>LED1</b> <button id="led1on" type="button" onclick="buttonclick(this);">On</button> <button id="led1off" type="button" onclick="buttonclick(this);">Off</button></div>
<div id="led2status"><b>LED2</b> <button id="led2on" type="button" onclick="buttonclick(this);">On</button> <button id="led2off" type="button" onclick="buttonclick(this);">Off</button></div>
<div id="led3status"><b>LED3</b> <button id="led3on" type="button" onclick="buttonclick(this);">On</button> <button id="led3off" type="button" onclick="buttonclick(this);">Off</button></div>
<div id="led4status"><b>LED4</b> <button id="led4on" type="button" onclick="buttonclick(this);">On</button> <button id="led4off" type="button" onclick="buttonclick(this);">Off</button></div>
</div>
</div>
</body>
</html>
)rawliteral";

const char * hostName = "esp-async";
const char* http_username = "admin";
const char* http_password = "admin";
const int LED1PIN = 12; //Tu definiujesz do jakiego pinu podlaczas przelacznik
const int LED2PIN = 13; //PIN ESP LED_BUILTIN zapala sie po wlaczeniu do masy, czyli w tym przykladzie podanie zera, OFF
const int LED3PIN = 15; //D4 to pin niebieskiej diody modulu ESP zapala się po zwarciu pinu do masy, czyli tu wciśniecie OFF
const int LED4PIN = 14;
uint8_t ledklik;
// Current LED status
bool LED1Status;
bool LED2Status;
bool LED3Status;
bool LED4Status;

// Commands sent through Web Socket
const char LED1ON[] = "led1on";
const char LED1OFF[] = "led1off";
const char LED2ON[] = "led2on";
const char LED2OFF[] = "led2off";
const char LED3ON[] = "led3on";
const char LED3OFF[] = "led3off";
const char LED4ON[] = "led4on";
const char LED4OFF[] = "led4off";
String webString = "", token="@&"; 
unsigned long previousMillis = 0;
unsigned long czasOdczytu= 1000;
uint16_t VAR1,VAR2,VAR3,VAR4;


static void writeLED1(bool);
static void writeLED2(bool);
static void writeLED3(bool);
static void writeLED4(bool);




 
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
AsyncEventSource events("/events"); 
void setup(){
pinMode(LED1PIN, OUTPUT);
  writeLED1(0);
  pinMode(LED2PIN, OUTPUT);
  writeLED4(1);
  pinMode(LED3PIN, OUTPUT);
  writeLED4(1);
  pinMode(LED4PIN, OUTPUT);
  writeLED4(1);
  #if (reczneIP) 
 WiFi.config(ip, dns, gateway, subnet);  
 #endif
  Serial.begin(115200);

 
  Serial.println();
  Serial.println();
  Serial.println();

 for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial.flush();
    delay(1000);
  } 


 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());

 //Send OTA events to the browser
  ArduinoOTA.onStart([]() { events.send("Update Start", "ota"); });
  ArduinoOTA.onEnd([]() { events.send("Update End", "ota"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    events.send(p, "ota");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if(error == OTA_AUTH_ERROR) events.send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) events.send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) events.send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) events.send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) events.send("End Failed", "ota");
  });
  ArduinoOTA.setHostname(hostName);
  ArduinoOTA.begin();
  

    if (!MDNS.begin("espWebSock")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");

     webSocket.begin();
 
  webSocket.onEvent(webSocketEvent); 

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", INDEX_HTML);
  });
   
  

  
  server.begin();
}
 
void loop(){
  webSocket.loop();
  ArduinoOTA.handle();
   unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= czasOdczytu)
  {
    VAR1++;
    VAR2+=2;
    VAR3+=3;
    VAR4+=4;
     webSocket.broadcastTXT("start"+token+String(VAR1)+token+String(VAR2)+token+String(VAR3)+token+String(VAR4)); 
    previousMillis=currentMillis;
  }
}




void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for(uint32_t i = 0; i < len; i++) {
    if(i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
       
        webString="start"+token;
        if (LED1Status) {
        webSocket.sendTXT(num, LED1ON, strlen(LED1ON));
         //  webString+=LED1ON+token;
        }
        else {
           webSocket.sendTXT(num, LED1OFF, strlen(LED1OFF));
          //  webString+=LED1OFF+token;
        }
        if (LED2Status) {
          webSocket.sendTXT(num, LED2ON, strlen(LED2ON));
        //    webString+=LED2ON+token;
        }
        else {
           webSocket.sendTXT(num, LED2OFF, strlen(LED2OFF));
         //   webString+=LED2OFF+token;
        }
        if (LED3Status) {
           webSocket.sendTXT(num, LED3ON, strlen(LED3ON));
        //   webString+=LED3ON+token;
        }
        else {
           webSocket.sendTXT(num, LED3OFF, strlen(LED3OFF));
          //  webString+=LED3OFF+token;
        }
        if (LED4Status) {
           webSocket.sendTXT(num, LED4ON, strlen(LED4ON));
        //    webString+=LED4ON+token;
        }
        else {
          webSocket.sendTXT(num, LED4OFF, strlen(LED4OFF));
          // webString+=LED4OFF+token;
        } 
        webString+=String(VAR1)+token+String(VAR2)+token+String(VAR3)+token+String(VAR4)+token;
         webSocket.sendTXT(num,webString);  //to dziala tylko przy otwieraniu nowego polaczenia               
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      ledziklik();
      if (strcmp(LED1ON, (const char *)payload) == 0) {
        writeLED1(true);
      }
      else if (strcmp(LED1OFF, (const char *)payload) == 0) {
        writeLED1(false);
      }
      else if (strcmp(LED2ON, (const char *)payload) == 0) {
        writeLED2(true);
      }
      else if (strcmp(LED2OFF, (const char *)payload) == 0) {
        writeLED2(false);
      }
      else if (strcmp(LED3ON, (const char *)payload) == 0) {
        writeLED3(true);
      }
      else if (strcmp(LED3OFF, (const char *)payload) == 0) {
        writeLED3(false);
      }
      else if (strcmp(LED4ON, (const char *)payload) == 0) {
        writeLED4(true);
      }
      else if (strcmp(LED4OFF, (const char *)payload) == 0) {
        writeLED4(false);
      }
      else {
        Serial.println("Unknown command");
      }
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
 
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

static void ledziklik()
{
ledklik++;
  }
//W zaleznosci od podlaczenia LED 1 moze ja zapalac lub gasic
static void writeLED1(bool LED1on)
{
  LED1Status = LED1on;
   
  if (LED1on) {
    digitalWrite(LED1PIN, 1);
  }
  else {
    digitalWrite(LED1PIN, 0);
  }
}

static void writeLED2(bool LED2on)
{
  LED2Status = LED2on;
   
  if (LED2on) {
    digitalWrite(LED2PIN, 1);
  }
  else {
    digitalWrite(LED2PIN, 0);
  }
}
static void writeLED3(bool LED3on)
{
  LED3Status = LED3on;
  
  if (LED3on) {
    digitalWrite(LED3PIN, 1);
  }
  else {
    digitalWrite(LED3PIN, 0);
  }
}

static void writeLED4(bool LED4on)
{
  LED4Status = LED4on;
   
  if (LED4on) {
    digitalWrite(LED4PIN, 1);
  }
  else {
    digitalWrite(LED4PIN, 0);
  }
}


