#pragma once

#include "feeder_pins.h"
#include "Arduino.h"

class Feeder
{
public:
	Feeder() {}

    void init(bool automaticFeedingLight);

    void process(unsigned long time);
    void startFeeding(unsigned long time);

    void checkFeederTimer(unsigned long time);
    void setFeedingIntervalEnabled(bool enabled) { feedingIntervalEnabled = enabled; }
    void setFeedInterval(unsigned long  interval) { feedInterval = interval; previousMillis = 0; }
    void setFeedingTime(unsigned long time) {feedingTime = time; }
    void setAutomaticFeedingLight(bool enabled) { automaticFeedingLight = enabled; }
    void setLightEnabled(bool enabled);

    bool getAutomaticFeedingLight() { return automaticFeedingLight; }
    
private:
    void feedingAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 180);
    void runFeeder(unsigned long time);

private:
    unsigned long previousMillis;       // ms
    unsigned int feedingTime;           // s
    bool feedingIntervalEnabled;
    unsigned long  feedInterval;        // ??? s

    bool feedEnable;
    unsigned long feedStartingTime;

    bool automaticFeedingLight;
    bool lightEnabled;
};