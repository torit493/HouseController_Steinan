#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <time.h>
#include "Clock.h"

//Pins used:############################################################
const int therm_servo = D1;                                           // pin for pressing temp
const int light_servo = D2;                                           // pin for servo pressing light
const int wake_up_hr  = 6;                                            // Hour at which to wake up
const int midday_hr   = 10;                                           // Clock hr equal/past this -> stops cirle heat, next heat @ wake_up_hr -> latest press @ (midday_hr-1):59 -> heat to 20
const int evening_hr  = 17;                                           // Turns on lights again at this hour
const int cyclic_press= 1;                                            // #hrs to wait between presses -> set to 1 as sometimes the press is not working, gdm shitty thermostat...
//######################################################################

//Physical parameters:##################################################
const int t_th        = 16;                                           // angle for pressing thermostat
const int t_tho       = 52;                                           // zero position for thermostat presser
const int l_th        = -20;                                          // angle needed to turn light on
const int l_tho       = 25;                                           // angle for zero_position light switch
const double omega    = 0.05;                                         // angular velocity of servo (deg/ms) google: 0.12s/60deg, real val = 0.5
//######################################################################

//Objects:##############################################################
Servo presser;                                                        // servo to press thermostat and lights
ESP8266WebServer server(80);                                          // server for ESP8266   default http port=80
Clock virtual_clock   = Clock();                                      // only used for thermostat values here...
HTTPClient http;
//######################################################################

//Variables for main loop control:######################################
int timezone          = 2*3600;                                       // +1 zone for time.h functions -> 1=winter, 2=summer
int dst               = 0;                                            // date swing time??? winter/summer i think? It was not adjusted, maybe if i had 1?...
boolean christmas     = false;                                        // condition for christmas theme on webpage
//######################################################################

//Webpage stuff:########################################################
//// Set your Static IP address, gateway and subnet (see control panel -> network status -> wifi details...)
//IPAddress local_IP(192, 168, 0, 195);
//IPAddress gateway(192, 168, 0, 1);
//IPAddress subnet(255, 255, 255, 0);

char* ssid            = "Torint_2.4";                                     //
char* password        = "88FQ338HFH";                                 //
String head    = "<html>"                                      //the start of a html page with grey background for the application.
                        "<head>"
                        "<style>"
                        "body{background-color:#444444; color:white;}"
                        "a:link{color:white;}"
                        "a:visited{color:yellow;}"
                        "</style>"
                        "</head>"
                        "<body>";
String christmas_head = "<html>"                                      //the start of a html page with grey background for the application.
                        "<head>"
                        "<style>"
                        "body{background-color:#444444; color:#47ab35;}"
                        "a:link{color:#fffb5e;}"
                        "a:visited{color:#e03939;}"
                        "</style>"
                        "</head>"
                        "<body>";
String ending         = "</body> </html>";                           // ends the document by closing the body and html parts
String coffee_URL     = "http://192.168.0.176";                       // URL to coffee machine on ESP32
String coffee_reply;                                                  // string for answer from coffeeMachine

String startup        = head + 
                        "This is the site controlling my thermostat. For info check <a href=\"/info\">/info</a>." 
                        + ending;
//######################################################################

//Various functions:####################################################
void blinkLight(int pin, int delayTime){
  digitalWrite(pin, HIGH);
  delay(delayTime);
  digitalWrite(pin, LOW);
  delay(delayTime);
  digitalWrite(pin, HIGH);
  delay(delayTime);
  digitalWrite(pin, LOW);
}

void zeroPosition(Servo aServo, int pin, double angle){
  aServo.attach(pin);
  for (int i = 0; i < 5; i++){
    aServo.write(angle);
    delay(1/omega);
  }
  aServo.detach();
}

// Main functions:######################################################
void info(){
  int hour = virtual_clock.getHours();
  int minutes = virtual_clock.getMinutes();
  String temp;
  if (christmas){
    temp = christmas_head + "<h2>Welcome to the webpage of the house! &#127877;</h2>"
            "<h3>&#10052; Clock: &#10052;</h3>" + hour + ":" + minutes +
            "<h3>&#127876; Alternatives: &#127876;</h3>";
  }else{
    temp = head + "<h2>Welcome to the webpage of the house!</h2>"
            "<h3>Clock: </h3>" + hour + ":" + minutes +
            "<h3>Alternatives: </h3>";
  }
  temp +="<a href=\"/lightsOn\">/lightsOn</a> &emsp;&emsp;-> turns lights on.<br>"
                "<a href=\"/lightsOff\">/lightsOff</a> &emsp;&ensp;&nbsp;-> turns lights off.<br>"
                "<a href=\"/heatOn\">/heatOn</a> &emsp;&emsp;&nbsp; -> turns heat on.<br>"
                "<a href=\"/heatOff\">/heatOff</a> &emsp;&emsp;&nbsp; -> turns heat off.<br>"
                "<a href=\"/dailyRoutine\">/dailyRoutine</a> &nbsp;-> sets schedule to heat@5 & light@6<br>"
                "<a href=\"/status\">/status</a> &nbsp;&emsp;&emsp;&emsp;-> information on scheduled heat and light<br>"
                "<a href=\"/brewCoffee\">/brewCoffee</a> &ensp; -> start coffee machine. This messes with the thermostat for some reason...<br>"
                "<a href=\"/changeFilter\">/changeFilter</a> &nbsp; -> opens/closes filter holder.<br>"
                "<a href=\"/off\">/off</a> &emsp;&emsp;&emsp;&emsp; -> everything is turned off.<br>"
                "<a href=\"/heatCycle\">/heatCycle</a> &emsp;&ensp;-> repress heat every 3h.<br>"
                "<a href=\"/morningLight\">/morningLight</a> -> light on 1h after heat.<br>"
                "<a href=\"/setTime\">/setTime</a> &emsp;&emsp;&nbsp; -> sets the clock from web.<br>"
                + ending;
  server.send(200,"text/html", temp);
}

void status(){
  String temp;
  if (christmas){
    temp = christmas_head;
  }else{
    temp = head;
  }

  temp = temp + "Clock set to: " + virtual_clock.getHours() + ":" + virtual_clock.getMinutes();
  if (virtual_clock.getHeater()){
    temp = temp + "<br>Set to heat at: " + virtual_clock.getHeatHours() + ":" + virtual_clock.getHeatMinutes() + ". ";
    if (virtual_clock.getTimedLight()){
      temp += " Morning light is ON. ";
    }
    if (virtual_clock.getHeatCycle() != 0){
      temp = temp + "<br>Heat set to cycle with intervals of: " + virtual_clock.getHeatCycle() + " hours. ";
    }
    if (virtual_clock.getHeating()){
      temp = temp + "<br>heat button was resently pressed.";
    }
  }else{
    temp = temp + "<br> No heating or light is scheduled. ";
  }
  if (christmas){
    temp += "&#127876;" + ending;
  }else{
    temp += ending;
  }
  
  server.send(200,"text/html", temp);
}

void setTime(){
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  virtual_clock.setTime(p_tm->tm_hour, p_tm->tm_min);

  int hour = virtual_clock.getHours();
  int minutes = virtual_clock.getMinutes();
  String temp = head + "The time is set to: " + hour + ":" + minutes + ending;
  server.send(200,"text/html", temp);
}

void lightsOn(){
  presser.attach(light_servo);
  for (int pos = l_tho; pos <= l_tho-l_th; pos++){
    presser.write(pos);
    delay(1/omega);
  }
  for(int pos = l_tho-l_th; pos >= l_tho; pos--){
    presser.write(pos);
    delay(1/omega);
  }
  presser.detach();

  String temp = head + "The lights are now ON." + ending;
  server.send(200,"text/html", temp);
}

void lightsOff(){
  presser.attach(light_servo);
  for (int pos = l_tho; pos >= l_tho+l_th; pos--){ // l_th = -20
    presser.write(pos);
    delay(1/omega);
  }
  for(int pos = l_tho+l_th; pos <= l_tho; pos++){
    presser.write(pos);
    delay(1/omega);
  }
  presser.detach();
  
  String temp = head + "The lights are now OFF." + ending;
  server.send(200,"text/html",temp);
}

void heatOn(){
  presser.attach(therm_servo);
  for (int i = 0; i < 2; i++){
    for (int pos = t_tho; pos <= t_tho+t_th; pos++){
      presser.write(pos);
      delay(1/omega);
    }
    for(int pos = t_tho+t_th; pos >= t_tho; pos--){
      presser.write(pos);
      delay(1/omega);
    }
  }
  presser.detach();

  String temp;
  int hour = virtual_clock.getHours() + 3;
  int minutes = virtual_clock.getMinutes();
  if (virtual_clock.getHeatCycle() == 3){
    temp = head + "The heater has been activated and will cycle on until stopped." + ending; 
  }else if (virtual_clock.getHeatCycle() != 0){
    temp = head + "The heater has been activated and cycle is on for: " + virtual_clock.getHeatCycle() + " hours. " + ending;
  }else{
    temp = head + "The heater has been activated and will turn off at " + hour + ":" + minutes + ". " + ending;
  }
  server.send(200,"text/html", temp);
}

void heatOff(){
  presser.attach(therm_servo);
  for (int pos = t_tho; pos >= t_tho-t_th; pos--){
    presser.write(pos);
    delay(1/omega);
  }
  for (int pos = t_tho-t_th; pos <= t_tho; pos++){
    presser.write(pos);
    delay(1/omega);
  }
  presser.detach();
  
  String temp = head + "The heater has been de-activated." + ending;
  server.send(200,"text/html", temp);
}

void dailyRoutine(){
  String temp;
  if(virtual_clock.getClock()){
    virtual_clock.setHeater(wake_up_hr-1,0);
    virtual_clock.setHeatCycle(24);
    virtual_clock.setLight(wake_up_hr,0);
    temp = head + "Daily routine mode activated! :D" + ending;
  }else{
    temp = head + "Set the clock first." + ending;
  }
  server.send(200,"text/html", temp);
}

void Off(){
  String temp = head + "Everything is turned off." + ending;
  virtual_clock.turnOffHeater();
  virtual_clock.setLight(0,0);
  server.send(200,"text/html", temp);
}

void morningLight(){
  String temp;
  if (virtual_clock.getHeater()){
    if (virtual_clock.getTimedLight()){
      virtual_clock.setLight(0, 0);
    temp = head + "Morning light OFF." + ending;
    }else{
      virtual_clock.setLight(virtual_clock.getHeatHours()+1, virtual_clock.getHeatMinutes());
    temp = head + "Morning light ON." + ending;
    }
  }else{
    temp = head + "Set the heat time first." + ending;
  }
  server.send(200, "text/html", temp);
}

void heatCycle(){
  String temp;
  if (virtual_clock.getHeatCycle() == 0){
    virtual_clock.setHeatCycle(1);
    temp = head + "Heat cycle on" + virtual_clock.getHeatCycle() +"h." + ending;
  } else{
    virtual_clock.setHeatCycle(0);
    temp = head + "Heat cycle turned off." + ending;
  }
  server.send(200,"text/html", temp);
}

void brewCoffee(){
  http.begin(coffee_URL + "/startFilling");  // sets address to open on GET command
  int httpCode = http.GET();                // opens URL and recieves code > 0 if OK, < 0 if error
  String temp;
  
  if (httpCode > 0){
    coffee_reply = http.getString();        // gets the string it would have written to a http
    temp = head + coffee_reply + ending;
    server.send(200, "text/html", temp);
  }else{
    temp = head + "error on coffee web, is it on?..." + ending;
    server.send(200, "text/html", temp);
  }

  http.end();
}

void changeFilter(){
  http.begin(coffee_URL + "/changeFilter");   // sets address to open on GET command
  int httpCode = http.GET();                // opens URL and recieves code > 0 if OK, < 0 if error
  String temp;
  
  if (httpCode > 0){
    coffee_reply = http.getString();        // gets the string it would have written to a http
    temp = head + coffee_reply + ending;
    server.send(200, "text/html", temp);
  }else{
    temp = head + "error on coffee web, is it on?..." + ending;
    server.send(200, "text/html", temp);
  }

  http.end();
}

void tvPower(){
  http.begin(coffee_URL + "/tvPower");      // sets address to open on GET command
  int httpCode = http.GET();                // opens URL and recieves code > 0 if OK, < 0 if error
  String temp;
  
  if (httpCode > 0){
    coffee_reply = http.getString();        // gets the string it would have written to a http
    temp = head + coffee_reply + ending;
    server.send(200, "text/html", temp);
  }else{
    temp = head + "error on coffee web, is it on?..." + ending;
    server.send(200, "text/html", temp);
  }

  http.end();
}
