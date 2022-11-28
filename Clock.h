#pragma once
#include "Arduino.h"

class Clock {                                     // clock with heater(/alarm) and light
private:
// CLOCK VARIABLES: -----------------------------------------------------------------------------------------------------------
  int hours, minutes;                             // the time is hours:minutes
  bool clock_set;                                 // has time been set?
  unsigned long set_time;                         // time when setting clock in seconds
// HEAT VARIABLES: ------------------------------------------------------------------------------------------------------------
  int heat_hrs, heat_min;                         // heat at heat_hrs:heat_min o'clock
  bool heater;                                    // is heat time turned on -> true, no set heat time -> false
  bool heating;                                   // has thermostat been pressed -> true, not pressed/still time left -> false
  int heat_cycle;                                 // hours to wait between each heat press, 0h/OFF by default
  unsigned long started_heat;                     // time at heating start
// LIGHT VARIABLES: -----------------------------------------------------------------------------------------------------------
  int light_hrs, light_min;                       // time to turn on lights
  int light_delay;                                // seconds to wait before turning on light after heat, 1h default in setup
  bool morning_light;                             // should light be turned on? -> true
  bool lights_on;                                 // has lights been pressed -> true, not pressed/still time left -> false
// ----------------------------------------------------------------------------------------------------------------------------

public:
// SETUP CLASS: ---------------------------------------------------------------------------------------------------------------
  Clock(int hours, int minutes);
  Clock(int c_hrs, int c_min, int h_hrs, int h_min);
  Clock() {hours=0; minutes=0; clock_set=false; set_time=millis()/1000; heat_hrs=0; heat_min=0; heater=false; heating=false; heat_cycle=0; light_hrs=0; light_min=0; morning_light=false; lights_on=false; light_delay=3600;};
// SET VARIABLES: -------------------------------------------------------------------------------------------------------------
  void setTime(int hours, int minutes);           // sets clock, clock_set=true, heater=false, heating=false
  void setHeater(int hours, int minutes);         // activates heater and sets time to heat
  void setHeatCycle(int cycle_hrs);               // set repeat time for heating, heater=true, heating=false
  void setLight(int hours, int minutes);          // sets time to turn on lights, morning_light=true if not 0.0, light_on=false
// GET VARIABLES: -------------------------------------------------------------------------------------------------------------
  int getHours() const;                           // returns clock hours
  int getMinutes() const;                         // returns clock minutes
  int getHeatHours() const;                       // returns heater hours
  int getHeatMinutes() const;                     // returns heater minutes
  int getLightHours() const;                      // returns light hours
  int getLightMinutes() const;                    // returns light minutes
  int getHeatCycle() const;                       // returns # hours for interval to press heater

  bool getClock() const;                          // return clock_set condition
  bool getHeater() const;                         // return heater condition
  bool getHeating() const;                        // return heating condition
  bool getTimedLight() const;                     // return morning_light condition
  bool lightsOn() const;                          //return lights_on
// ACTING CONDITIONS: ---------------------------------------------------------------------------------------------------------
  bool timeForHeat();                             // true as clock turns heat time, heating=true, sets heating false after one minute past heat time.
  bool timeForLight();                            // true as clock turns light time, lights_on=true
  void turnOffHeater();                           // heat hrs&min=0, heater=false, heating=false
  void turnOffLighting();                         // light hrs&min=0, morning_light=false, lights_on=false;
  void switchLights();                            // changes lights_on condition
};
