#pragma once

enum CommandType {
    RunFishFeeder,
    FeedingIntervalEnabled,
    SetFeedingInterval,
    SetFeedingTime,
    SetAutomaticLight,
    SetLight,
    SaveFeederSettings,
    None
};

typedef struct {
        CommandType command; 
        int value;
} command_t;

extern command_t event; // TODO remove this global variable and move it inside checkCommnad function

extern void feederCheckCommand();
extern char* feederGetValues(char * p);