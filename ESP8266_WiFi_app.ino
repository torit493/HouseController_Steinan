#include "func.h"

//###############################################//
//## Choose board NodeMCU 1.0 (ESP-12E Module) ##//
//###############################################//

void setup() {
  Serial.begin(115200);
  zeroPosition(presser, light_servo, l_tho);
  zeroPosition(presser, therm_servo, t_tho);
  
//  if (!WiFi.config(local_IP, gateway, subnet)) {
//    Serial.println("STA Failed to configure");
//  }else{
//    Serial.println("connecting to assigned IP...");
//  }
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Adress: ");
  Serial.print(WiFi.localIP());

  server.on("/", info);
  server.on("/lightsOn", lightsOn);
  server.on("/lightsOff", lightsOff);
  server.on("/heatOn", heatOn);
  server.on("/heatOff", heatOff);
  server.on("/dailyRoutine", dailyRoutine);
  server.on("/status", status);
  server.on("/brewCoffee", brewCoffee);
  server.on("/changeFilter", changeFilter);
  server.on("/tvPower", tvPower);
  server.on("/off", Off);
  server.on("/heatCycle", heatCycle);
  server.on("/morningLight", morningLight);
  server.on("/setTime", setTime);
  server.begin();

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");

  while(!time(nullptr)){
    Serial.println("wtf...?");
    delay(1000);
  }
  
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  virtual_clock.setTime(p_tm->tm_hour, p_tm->tm_min);
  Serial.println("");
  Serial.print(virtual_clock.getHours());
  Serial.print(":");
  Serial.println(virtual_clock.getMinutes());
}

void loop() {
  server.handleClient();
  
  if (virtual_clock.timeForHeat()){
    heatOn();
    int curr_hr = virtual_clock.getHours();
    if (curr_hr == wake_up_hr-1 && virtual_clock.getHeatCycle() == 24){                                       // turns on heat in morning
      virtual_clock.setHeatCycle(cyclic_press);                                                               // switches to 1hr cycle, as sometimes unable to press...
    }else if (curr_hr >= midday_hr && curr_hr < midday_hr+1 && virtual_clock.getHeatCycle() == cyclic_press){ // in evening switch to 24hr cycle, with press at morning, turns on morning light
      virtual_clock.setHeater(wake_up_hr-1,0);
      virtual_clock.setHeatCycle(24);
      heatOff();
      //lightsOff();
      virtual_clock.setLight(wake_up_hr,0);
    }else if (curr_hr == evening_hr){
      lightsOn();
    }
  }
  if (virtual_clock.timeForLight()){
    lightsOn();                                                                                 // turns lights on
    virtual_clock.setLight(0,0);                                                                // only do this once in morning
    //brewCoffee();
  }
}
