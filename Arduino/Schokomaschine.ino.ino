/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//Structure
// CMD1:  keep? (or goto)  |   temperature | until button? (or time)   | time | active
// CMD2:
// CMD3:
int commands[15][5];
int linesLength = 15;
int cmdLength = 5;
int cmdPos = 0;


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define heater 13
#define led 12
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

ESP8266WiFiMulti wifiMulti;

unsigned long nextEvent = 0;
int isRunning = 0;
int gotoDirection = 1; // 1 = goDownTo -1 = goUpTo
String profileName = "a";

MDNSResponder mdns;

ESP8266WebServer server ( 80 );

void handleRoot() {
	char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

		hr, min % 60, sec % 60
	);
  
	server.send ( 200, "text/html", temp );
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void setup ( void ) {
	pinMode ( heater, OUTPUT );
  pinMode ( led, OUTPUT );
	digitalWrite ( heater, 0 );
	Serial.begin ( 9600 );

  delay(10);
  
  wifiMulti.addAP("Csiky 2.0", "mediasch");
  wifiMulti.addAP("Csiky", "mediasch");
  wifiMulti.addAP("UPC1671076", "TNBAZBHX");
  wifiMulti.addAP("handy", "        ");
	Serial.println ( "" );

  digitalWrite(led, 1);
	// Wait for connection
	while ( wifiMulti.run() != WL_CONNECTED ) {
		delay ( 250 );
    digitalWrite(led, 0);
    delay(250);
    digitalWrite(led, 1);
		Serial.print ( "." );
	}
  digitalWrite(led, 0);

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( mdns.begin ( "schokomaschine", WiFi.localIP() ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on(  "/",        handleRoot );
  server.on(  "/profile", profileGet );
  server.on(  "/profile/temp",    profileGetTemp);
  server.on(  "/profile/clear", profileClear);
  server.on(  "/profile/add/keep/button", profileAddKeepTempButton);
  server.on(  "/profile/add/keep/time", profileAddKeepTempTime);
  server.on(  "/profile/add/goto", profileAddGotoTemp);
  server.on(  "/profile/run", profileRun);
  server.on(  "/profile/finishStep", profileFinishStep);
  server.on(  "/profile/gotoStep", profileGotoStep);
  
	server.onNotFound ( handleNotFound );
 
	server.begin();
	Serial.println ( "HTTP server started" );
  sensors.begin();
}


void loop ( void ) {
	mdns.update();
	server.handleClient();
  int isKeep = commands[cmdPos][0];
  int temperature = commands[cmdPos][1];
  int isUntilButton = commands[cmdPos][2];
  int untilTime = commands[cmdPos][3];
  int profileStatus = commands[cmdPos][4];
  //Is it on and running?
  if((profileStatus == 1 || profileStatus == 2) && isRunning){
    Serial.println ( "profiler working" );
    if(isKeep){
      //KEEP TEMP UNTIL..

      //is unitl Button
      if(isUntilButton){
        Serial.println ( "Keeping temp until button press" );
        //KEEP UNTIL BUTTON
        if(profileStatus == 1){
          commands[cmdPos][4] = 2; //set Status to running
          commands[cmdPos][3] = (int) millis();
        }
        
        if(currTemp() > temperature){
          Serial.println("cooling down..");
          digitalWrite(heater, LOW);
        }else{
          Serial.println("heating up...");
          digitalWrite(heater, HIGH); 
        }
        
      }else{
        Serial.println ( "Keeping Temp until time" );
        //KEEP UNITL TIME
        if(profileStatus == 1){
          commands[cmdPos][4] = 2; //set Status to running
          nextEvent = (millis() + ((long) untilTime));
        }
        if(currTemp() > temperature){
          digitalWrite(heater, LOW);
        }else{
          digitalWrite(heater, HIGH); 
        }
        
        //should end?
        if(nextEvent <= millis()){
          commands[cmdPos][4] = 3; // set Stauts to ended
          ++cmdPos;
        }
      }
    }else{//is not keep; is goto
      Serial.println ( "Goto temp" );
      //GOTO TEMP
      if(profileStatus == 1){
        commands[cmdPos][4] = 2; //set Status to running
        Serial.print(currTemp());
        Serial.print("  ");
        Serial.print(currTemp());
        Serial.print("  ");
        Serial.print(currTemp());
        Serial.print("  ");
        Serial.print(currTemp());
        Serial.print(" vs ");
        Serial.print(temperature);
        Serial.println(";");
        if(currTemp() > temperature){
          gotoDirection = -1;
          
          Serial.println ( "goto Temp direction is -1" );
        }else{
          gotoDirection = 1;
          Serial.println ( "goto Temp direction is 1" );  
        }
       
      }
      //did it reach the temperature?
      Serial.println("Did is reach the temp?");
      if((currTemp() - temperature)*gotoDirection > 0){
        Serial.println("Yes going to next cmd");
        commands[cmdPos][4] = 3;
        digitalWrite(heater, LOW);
        ++cmdPos; 
      }else{
          Serial.println("no");
         if(currTemp() > temperature){
          Serial.println("heater off");
          digitalWrite(heater, LOW);
         }else{
          Serial.println("heater on");
          digitalWrite(heater, HIGH);
         }
      }
    }  
  }
}

String commandsToString(){
  String str = "";
  for(int i = 0; i < linesLength; i++){
    for(int j = 0; j < cmdLength; j++){
       str += commands[i][j];
       str += ",";
    }
    str += ';';
  }
  return str;
}

int currTemp(){
  sensors.requestTemperatures();
  return int(sensors.getTempCByIndex(0)*100);
}


// Profile...
// CMD Structure
// CMD1:  keep? (or goto)  |   temperature | until button? (or time)   | time | active?(0 = off, 1 = firstTime, 2 = started, 3 = finished)
// CMD2:
// CMD3:

void profileGet(){
  String ret = "{\"currTemp\": ";
  ret += String(currTemp());
  ret += ",";
  ret += "\"nextEvent\": ";
  ret += String(nextEvent);
  ret += ",";
  ret += "\"profileName\": \"";
  ret += profileName;
  ret += "\", ";
  ret += "\"commands\": \"";
  ret += commandsToString();
  ret += "\"}";
  server.send ( 200, "text/plain", ret);
}
void profileGetTemp(){
  server.send ( 200, "text/plain", String(currTemp()));
}


void profileClear(){
  
  for(int i = 0; i < linesLength; ++i){
    for(int j = 0; j < cmdLength; ++j){
      commands[i][j] = 0;  
    }  
  }
  cmdPos = 0;
  isRunning = 0;
  profileName = "";
  server.send ( 200, "text/plain", commandsToString());
}



void profileAddKeepTempButton(){
  int temp = server.arg(0).toInt();
  commands[cmdPos][0] = 1;
  commands[cmdPos][1] = temp;
  commands[cmdPos][2] = 1;
  commands[cmdPos][3] = 0;
  commands[cmdPos][4] = 1;
  
  server.send ( 200, "text/plain", commandsToString());
  ++cmdPos;
}

void profileAddKeepTempTime(){
  int temp = server.arg(0).toInt();
  int duration = server.arg(1).toInt();
  commands[cmdPos][0] = 1;
  commands[cmdPos][1] = temp;
  commands[cmdPos][2] = 0;
  commands[cmdPos][3] = duration;
  commands[cmdPos][4] = 1;
  
  server.send ( 200, "text/plain", commandsToString());
  ++cmdPos;
}

void profileAddGotoTemp(){
  int temp = server.arg(0).toInt();
  commands[cmdPos][0] = 0;
  commands[cmdPos][1] = temp;
  commands[cmdPos][2] = 0;
  commands[cmdPos][3] = 0;
  commands[cmdPos][4] = 1;
  
  server.send ( 200, "text/plain", commandsToString());
  ++cmdPos;
}

void profileRun(){
  cmdPos = 0;
  isRunning = 1;
  profileName = String(server.arg(0));
  server.send ( 200, "text/plain", commandsToString());
  
  
  Serial.print("Run Profile!");
  Serial.println(isRunning);
}

void profileFinishStep(){
  commands[cmdPos][4] = 2;
  server.send ( 200, "text/plain", commandsToString());
}

void profileGotoStep(){
  cmdPos = server.arg(0).toInt();
  server.send ( 200, "text/plain", commandsToString());
}
