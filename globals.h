#pragma once

enum CommandType {
    RunFishFeeder,
    FeedingIntervalEnabled,
    SetFeedingInterval,
    SetFeedingTime,
    SetAutomaticLight,
    SetLight,
    None
};

typedef struct {
        CommandType command; 
        int value;
} command_t;

// TODO remove these globals 
extern unsigned long previousMillis;   // ms
extern unsigned long feedinterval;     // s
extern unsigned int feedingtime;       // s
extern bool feedingintervalenabled;
extern bool lightEnabled;

extern command_t event; // TODO remove this global variable and move it inside checkCommnad function

extern void checkCommand();
extern char* getValues(char * p);