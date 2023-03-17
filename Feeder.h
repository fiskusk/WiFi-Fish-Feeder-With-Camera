#pragma once

#include "feeder_pins.h"
#include "Arduino.h"
#include <Preferences.h>
#include "time.h"

class Feeder
{
public:
	Feeder() {}

    void init();

    void process(unsigned long time);
    void startFeeding(unsigned long time);
    void saveDefaults();

    void setFeedingIntervalEnabled(bool enabled) { feedingIntervalEnabled = enabled; }
    void setFeedInterval(unsigned long  interval) { feedInterval = interval; previousMillis = 0; }
    void setFeedingTime(unsigned long time) {feedingTime = time; }
    void setAutomaticFeedingLight(bool enabled) { automaticFeedingLight = enabled; }
    void setLightEnabled(bool enabled);

    bool getAutomaticFeedingLight() { return automaticFeedingLight; }
    bool getLightEnabled() { return lightEnabled; }
    bool getFeedingIntervalEnabled() { return feedingIntervalEnabled; }
    unsigned long getFeedingInterval() { return feedInterval; }
    unsigned long getFeedingTime() { return feedingTime; }
    char* getLastFeedingTime();
    
private:
    void feedingAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 180);
    void runFeeder(unsigned long time);
    void checkFeederTimer(unsigned long time);

private:
    Preferences preferences;
    unsigned long previousMillis;       // ms
    unsigned int feedingTime;           // s
    bool feedingIntervalEnabled;
    unsigned long  feedInterval;        // ??? s

    bool feedEnable;
    unsigned long feedStartingTime;

    bool automaticFeedingLight;
    bool lightEnabled;

    char lastFeedingTime[256];
};