#include "Feeder.h"

void Feeder::init(bool automaticFeedingLight)
{
    // --- Start header changes for Fish Feeder ---
    previousMillis =    0;   // ms
    feedInterval =      24UL*60UL*60UL*1000UL;
    feedingTime =       2;       // s
    feedingIntervalEnabled = true;

    feedEnable = false;
    feedStartingTime = 0;

    this->automaticFeedingLight = automaticFeedingLight;
    lightEnabled = false;

    // Feeder onboard Light
    pinMode(PWR_LED_NUM, OUTPUT);

    // Feeder servo PWM
    pinMode(SERVO_NUM, OUTPUT);
    
    ledcSetup(SERVO_CHANNEL, 50, 16);         // channel, freq, resolution
    ledcAttachPin(SERVO_NUM, SERVO_CHANNEL);  // pin, channel    
}

void Feeder::checkFeederTimer(unsigned long time)
{
    if (!feedingIntervalEnabled)
        return;

    if (time - previousMillis >= feedInterval) {
        previousMillis += feedInterval;

        Serial.println("Feeder timer activated.");
        startFeeding(time);
    }  
}

void Feeder::startFeeding(unsigned long time)
{
    Serial.print("Start feeding the fish for ");
    Serial.print(feedingTime);
    Serial.println(" seconds");
    feedStartingTime = time;
    feedEnable = true;
    if (automaticFeedingLight)
        digitalWrite(PWR_LED_NUM , HIGH);
    feedingAnalogWrite(4, 90);
}

void Feeder::runFeeder(unsigned long time)
{
        if (time > feedStartingTime + feedingTime * 1000) {
            feedingAnalogWrite(4, 0); 
            if (automaticFeedingLight && lightEnabled == false) {
                digitalWrite(PWR_LED_NUM , LOW);
            }
            Serial.println("Feeding the fish finished");
            feedEnable = false;
        }
        
        /*
        for (uint8_t sweep = 0; sweep < 180; sweep++)
        {
            feedingAnalogWrite(4, sweep);
            delay((feedingtime * 1000) / 180);
            Serial.print("Sweep ");
            Serial.print(sweep);
            Serial.println(" now");
        }
        */
}

void Feeder::feedingAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax) 
{
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t duty = (8191 / valueMax) * min(value, valueMax);
    ledcWrite(channel, duty);
}

void Feeder::setLightEnabled(bool enabled) 
{
    lightEnabled = enabled;
    digitalWrite(PWR_LED_NUM , enabled);
}

void Feeder::process(unsigned long time)
{
    checkFeederTimer(time);
    if (feedEnable == true) { runFeeder(time); }
}

