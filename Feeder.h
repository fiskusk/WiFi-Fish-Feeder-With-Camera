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
    void calculateTimeBetweenFeeding();

    void setFeedingIntervalEnabled(bool enabled) { feedingIntervalEnabled = enabled; }
    void setFeedInterval(uint8_t interval) { feedInterval = interval; calculateTimeBetweenFeeding(); }
    void setFeedingTime(unsigned long time) {feedingTime = time; }
    void setAutomaticFeedingLight(bool enabled) { automaticFeedingLight = enabled; }
    void setLightEnabled(bool enabled);

    void setFirstFeedDateTime(String firstFeed);
    void setSecondFeedDateTime(String secondFeed);
    void setThirdFeedDateTime(String thirdFeed);
    void setFourthFeedDateTime(String fourthFeed);

    String getFirstFeedDateTimeString() { return msToTimeHhMmSs(firstFeedingMs); };
    String getSecondFeedDateTimeString() { return msToTimeHhMmSs(secondFeedingMs); };
    String getThirdFeedDateTimeString() { return msToTimeHhMmSs(thirdFeedingMs); };
    String getFourthFeedDateTimeString() { return msToTimeHhMmSs(fourthFeedingMs); };

    bool getAutomaticFeedingLight() { return automaticFeedingLight; }
    bool getLightEnabled() { return lightEnabled; }
    bool getFeedingIntervalEnabled() { return feedingIntervalEnabled; }
    uint8_t getFeedingInterval() { return feedInterval; }
    unsigned long getFeedingTime() { return feedingTime; }
    char* getLastFeedingTime();

    void setFeedingCalendarEnabled(bool enabled) { feedCalendarEnabled = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnMonday(bool enabled) { feedOnMonday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnTuesday(bool enabled) { feedOnTuesday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnWednesday(bool enabled) { feedOnWednesday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnThursday(bool enabled) { feedOnThursday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnFriday(bool enabled) { feedOnFriday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnSaturday(bool enabled) { feedOnSaturday = enabled; calculateTimeBetweenFeeding(); }
    void setFeedingOnSunday(bool enabled) { feedOnSunday = enabled; calculateTimeBetweenFeeding(); }

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
    tm parseTimeString(String string);
    unsigned long timeHhMmSsToMs(tm time);
    String msToTimeHhMmSs(unsigned long timeMs);

private:
    Preferences preferences;
    Preferences feederCalendar;
    unsigned long previousMillis;       // ms
    unsigned long timeBetweenFeeding;       // ms
    unsigned int feedingTime;           // s
    bool feedingIntervalEnabled;
    uint8_t feedInterval;        // ??? s

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

    unsigned long firstFeedingMs;
    unsigned long secondFeedingMs;
    unsigned long thirdFeedingMs;
    unsigned long fourthFeedingMs;

};