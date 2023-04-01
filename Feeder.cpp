#include "Feeder.h"

void Feeder::init()
{
    preferences.begin("feeder", false);
    // --- Start header changes for Fish Feeder ---
    feedInterval = preferences.getULong("feedInterval", 12UL);
    feedingTime = preferences.getUInt("feedingTime", 2);
    feedingIntervalEnabled = preferences.getBool("feedIntervalEn", true);
    lightEnabled = preferences.getBool("lightEnabled", false);
    automaticFeedingLight = preferences.getBool("autoFeedLight", true);
    preferences.end();

    previousMillis =    0;   // ms
    feedEnable = false;
    feedStartingTime = 0;

    strcpy(lastFeedingTime, "Unknown");

    // Feeder onboard Light
    pinMode(PWR_LED_NUM, OUTPUT);

    // Feeder servo PWM
    pinMode(SERVO_NUM, OUTPUT);
    
    ledcSetup(SERVO_CHANNEL, 50, 16);         // channel, freq, resolution
    ledcAttachPin(SERVO_NUM, SERVO_CHANNEL);  // pin, channel    
}

void Feeder::saveDefaults()
{
    preferences.begin("feeder", false);

    preferences.putULong("feedInterval", feedInterval);
    preferences.putUInt("feedingTime", feedingTime);
    preferences.putBool("feedIntervalEn", feedingIntervalEnabled);
    preferences.putBool("lightEnabled", lightEnabled);
    preferences.putBool("autoFeedLight", automaticFeedingLight);

    Serial.print("Stored feedInterval: ");
    Serial.println(preferences.getULong("feedInterval") );
    Serial.print("Stored feedingTime: ");
    Serial.println(preferences.getUInt("feedingTime") );
    Serial.print("Stored feedIntervalEn: ");
    Serial.println(preferences.getBool("feedIntervalEn") );
    Serial.print("Stored lightEnabled: ");
    Serial.println(preferences.getBool("lightEnabled") );
    Serial.print("Stored autoFeedLight: ");
    Serial.println(preferences.getBool("autoFeedLight") );

    Serial.println("Current feeding settings saved");

    preferences.end();
}

char* Feeder::getLastFeedingTime()
{
    char* str = (char*)malloc(256 * sizeof(char));
    strcpy(str, lastFeedingTime);
    return str;
}

void Feeder::checkFeederTimer(unsigned long time)
{
    if (!feedingIntervalEnabled)
        return;

    if (time - previousMillis >= feedInterval*60UL*60UL*1000UL) {
        previousMillis += feedInterval*60UL*60UL*1000UL;

        Serial.println("Feeder timer activated.");
        startFeeding(time);
    }  
}

void Feeder::startFeeding(unsigned long time)
{
    struct tm timeinfo;
    char timeStringBuff[256];
    if(!getLocalTime(&timeinfo, 10)){
      Serial.println("Failed to obtain time");
      //printf(timeStringBuff, "Unknown");
    }
    //else{
    //    //Serial.println(&timeinfo, "%A, %d.%B %Y %H:%M:%S");   
    //}
    strftime(timeStringBuff, sizeof(timeStringBuff), "%a, %d.%b %Y %H:%M:%S", &timeinfo); 

    memcpy(lastFeedingTime, timeStringBuff, sizeof(timeStringBuff));

    Serial.print("Start feeding the fish for ");
    Serial.print(feedingTime);
    Serial.print(" seconds at ");
    Serial.println(lastFeedingTime);
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

