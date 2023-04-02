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
    void saveFeederCalendar();

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

    void setFeedingCalendarEnabled(bool enabled) { feedCalendarEnabled = enabled; }
    void setFeedingOnMonday(bool enabled) { feedOnMonday = enabled; }
    void setFeedingOnTuesday(bool enabled) { feedOnTuesday = enabled; }
    void setFeedingOnWednesday(bool enabled) { feedOnWednesday = enabled; }
    void setFeedingOnThursday(bool enabled) { feedOnThursday = enabled; }
    void setFeedingOnFriday(bool enabled) { feedOnFriday = enabled; }
    void setFeedingOnSaturday(bool enabled) { feedOnSaturday = enabled; }
    void setFeedingOnSunday(bool enabled) { feedOnSunday = enabled; }

    bool getFeedingCalendarEnabled() { return feedCalendarEnabled; }
    bool getFeedingOnMonday() { return feedOnMonday; }
    bool getFeedingOnTuesday() { return feedOnTuesday; }
    bool getFeedingOnWednesday() { return feedOnWednesday; }
    bool getFeedingOnThursday() { return feedOnThursday; }
    bool getFeedingOnFriday() { return feedOnFriday; }
    bool getFeedingOnSaturday() { return feedOnSaturday; }
    bool getFeedingOnSunday() { return feedOnSunday; }
    
private:
    void feedingAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 180);
    void runFeeder(unsigned long time);
    void checkFeederTimer(unsigned long time);

private:
    Preferences preferences;
    Preferences feederCalendar;
    unsigned long previousMillis;       // ms
    unsigned int feedingTime;           // s
    bool feedingIntervalEnabled;
    unsigned long  feedInterval;        // ??? s

    bool feedEnable;
    unsigned long feedStartingTime;

    bool automaticFeedingLight;
    bool lightEnabled;

    char lastFeedingTime[256];

    bool feedCalendarEnabled;
    bool feedOnMonday;
    bool feedOnTuesday;
    bool feedOnWednesday;
    bool feedOnThursday;
    bool feedOnFriday;
    bool feedOnSaturday;
    bool feedOnSunday;
};