#pragma once

enum CommandType {
    SetAutomaticLight,
    SetLight, 
    None
};

typedef struct {
        CommandType command; 
        int value;
} command_t;

extern unsigned long previousMillis;   // ms
extern unsigned long feedinterval;     // s
extern unsigned int feedingtime;       // s
extern bool feedingintervalenabled;
extern bool lightEnabled;
extern bool automaticFeedingLight;

extern command_t event;