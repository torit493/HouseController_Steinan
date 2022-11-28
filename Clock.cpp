#include "Clock.h"

// SETUP CLASS: ---------------------------------------------------------------------------------------------------------------
Clock::Clock(int hours, int minutes) {
  this->hours = hours;
  this->minutes = minutes;
  clock_set = true;
  set_time = millis()/1000;
  
  heat_hrs,heat_min = 0;
  heat_cycle = 0;
  heater, heating = false;

  light_hrs,light_min = 0;
  morning_light, lights_on = false;
  light_delay = 3600;
}

Clock::Clock(int c_hrs, int c_min, int h_hrs, int h_min){
  hours = c_hrs;
  minutes = c_min;
  clock_set = true;
  set_time = millis()/1000;
  
  heat_hrs = h_hrs;
  heat_min = h_min;
  heat_cycle = 0;
  heater = true;
  heating = false;
  started_heat = millis()/1000;           // NOT USED??

  light_hrs, light_min = 0;
  morning_light = false;
  light_delay = 3600;
}

// SET VARIABLES: -------------------------------------------------------------------------------------------------------------
void Clock::setTime(int hours, int minutes) {
  this->hours = hours;
  this->minutes = minutes;
  clock_set = true;
  set_time = millis()/1000;

  heater = false;
  heating = false;
}

void Clock::setHeater(int hours, int minutes){
  heat_hrs = hours;
  heat_min = minutes;
  heater = true;
  heating = false;
  started_heat = millis()/1000;           // NOT USED??
  
  light_hrs = hours + light_delay/3600;
  light_min = minutes;
}

void Clock::setHeatCycle(int cycle_hrs){
  if (!heater){
    heat_hrs = getHours();
    heat_min = getMinutes();
    heater = true;
  }
  heat_cycle = cycle_hrs;
  heating = false;
}

void Clock::setLight(int hours, int minutes) {
  light_hrs = hours;
  light_min = minutes;
  light_delay = (hours-heat_hrs)*3600 + (minutes-heat_min)*60;
  if(hours == 0 && minutes == 0){
    morning_light = false;
  }else{
    morning_light = true;
  }
  lights_on = false;
}

// GET VARIABLES: -------------------------------------------------------------------------------------------------------------
int Clock::getHours() const{
  int real_h;
  long int real_minutes = hours*60 + minutes + millis()/1000/60 - set_time/60;
  if (real_minutes < 60){
    real_h = 0;
  }else{
    real_h = (real_minutes - real_minutes%60)/60;
    while (real_h >= 24){
      real_h -= 24;
    }
  }
  return real_h;
}

int Clock::getMinutes() const{
  int real_m;
  long int real_minutes = hours*60 + minutes + millis()/1000/60 - set_time/60;
  if (real_minutes < 60){
    real_m = real_minutes;
  }else{
    real_m = real_minutes%60;
  }
  return real_m;
}

int Clock::getHeatHours() const{
  return heat_hrs;
}

int Clock::getHeatMinutes() const{
  return heat_min;
}

int Clock::getLightHours() const{
  return light_hrs;
}

int Clock::getLightMinutes() const{
  return light_min;
}

bool Clock::getClock() const{
  return clock_set;
}

bool Clock::getHeater() const{
  return heater;
}

bool Clock::getHeating() const{
  return heating;
}

bool Clock::getTimedLight() const{
  return morning_light;
}

bool Clock::lightsOn() const{
  return lights_on;
}

int Clock::getHeatCycle() const{
  return heat_cycle;
}

// ACTING CONDITIONS: ---------------------------------------------------------------------------------------------------------
bool Clock::timeForHeat(){
  if(heating && getMinutes() == heat_min+1){
    heating = false;
    Serial.println("reset heat");
  }
  if(heater && getHours() == heat_hrs && getMinutes() == heat_min && !heating) {      // must use get func as this adds the passed time, hours and minutes simply keep const int of startup time!
    heating = true;
    Serial.println("now heat");
    if (heat_cycle > 0){
      if (heat_hrs + heat_cycle < 24){
        heat_hrs += heat_cycle;
      }else{
        heat_hrs += heat_cycle - 24;
      }
    }else{
      heater = false;
    }
    return true;
  }else{
    return false;
  }
}

bool Clock::timeForLight(){
  if(lights_on && getMinutes() == light_min+1){
    lights_on = false;
  }
  if(morning_light && getHours() == light_hrs && getMinutes() == light_min && !lights_on) {
    lights_on = true;
    if (heat_cycle > 0){
      if (heat_hrs + heat_cycle < 24){
        light_hrs += heat_cycle;
      }else{
        light_hrs += heat_cycle - 24;
      }
    }
    return true;
  }else{
    return false;
  }
}

void Clock::turnOffHeater(){
  heat_hrs = 0;
  heat_min = 0;
  heat_cycle = 0;
  heater = false;
  heating = false;
}

void Clock::turnOffLighting(){
  light_hrs = 0;
  light_min = 0;
  morning_light = false;
  lights_on = false;
}

void Clock::switchLights(){
  if (lights_on){
    lights_on = false;
  }else{
    lights_on = true;
  }
}
