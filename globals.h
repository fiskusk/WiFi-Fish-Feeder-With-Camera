#pragma once

enum CommandType {
    RunFishFeeder,
    FeedingIntervalEnabled,
    SetFeedingInterval,
    SetFeedingTime,
    SetAutomaticLight,
    SetLight,
    SaveFeederSettings,
    SaveFeederCalendar,
    SetFeederCalendarEnabled,
    SetFirstFeed,
    SetSecondFeed,
    SetThirdFeed,
    SetFourthFeed,
    SetFeedOnMonday,
    SetFeedOnTuesday,
    SetFeedOnWednesday,
    SetFeedOnThursday,
    SetFeedOnFriday,
    SetFeedOnSaturday,
    SetFeedOnSunday,
    None
};

typedef struct {
        CommandType command; 
        int valueInt;
        String valueString;
} command_t;

extern void feederCheckCommand(command_t event);
extern char* feederGetValues(char * p);