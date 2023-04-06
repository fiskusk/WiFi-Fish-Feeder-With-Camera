#include "Feeder.h"

void Feeder::init()
{
    preferences.begin("feeder", false);
    // --- Start header changes for Fish Feeder ---
    feedInterval = preferences.getUShort("feedInterval", 2);
    feedingTime = preferences.getUInt("feedingTime", 2);
    feedingIntervalEnabled = preferences.getBool("feedIntervalEn", true);
    lightEnabled = preferences.getBool("lightEnabled", false);
    automaticFeedingLight = preferences.getBool("autoFeedLight", true);
    firstFeedDateTime = preferences.getString("firstFeed", "07:00:00");
    preferences.end();

    feederCalendar.begin("feederCalendar", false);
    feedCalendarEnabled = feederCalendar.getBool("feedCalEn", true);
    feedOnMonday = feederCalendar.getBool("feedOnMon", true);
    feedOnTuesday = feederCalendar.getBool("feedOnTue", true);
    feedOnWednesday = feederCalendar.getBool("feedOnWed", true);
    feedOnThursday = feederCalendar.getBool("feedOnThu", true);
    feedOnFriday = feederCalendar.getBool("feedOnFri", true);
    feedOnSaturday = feederCalendar.getBool("feedOnSat", false);
    feedOnSunday = feederCalendar.getBool("feedOnSun", true);
    feederCalendar.end();

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

    preferences.putUShort("feedInterval", feedInterval);
    preferences.putUInt("feedingTime", feedingTime);
    preferences.putBool("feedIntervalEn", feedingIntervalEnabled);
    preferences.putBool("lightEnabled", lightEnabled);
    preferences.putBool("autoFeedLight", automaticFeedingLight);

    Serial.print("Stored feedInterval: ");
    Serial.println(preferences.getUShort("feedInterval") );
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

void Feeder::saveFeederCalendar()
{
    feederCalendar.begin("feederCalendar", false);

    feederCalendar.putBool("feedCalEn", feedCalendarEnabled);
    feederCalendar.putBool("feedOnMon", feedOnMonday);
    feederCalendar.putBool("feedOnTue", feedOnTuesday);
    feederCalendar.putBool("feedOnWed", feedOnWednesday);
    feederCalendar.putBool("feedOnThu", feedOnThursday);
    feederCalendar.putBool("feedOnFri", feedOnFriday);
    feederCalendar.putBool("feedOnSat", feedOnSaturday);
    feederCalendar.putBool("feedOnSun", feedOnSunday);

    Serial.print("Stored feedCalendarEnabled: ");
    Serial.println(feederCalendar.getBool("feedCalEn") );
    Serial.print("Stored feedOnMonday: ");
    Serial.println(feederCalendar.getBool("feedOnMon") );
    Serial.print("Stored feedOnTuesday: ");
    Serial.println(feederCalendar.getBool("feedOnTue") );
    Serial.print("Stored feedOnWednesday: ");
    Serial.println(feederCalendar.getBool("feedOnWed") );
    Serial.print("Stored feedOnThursday: ");
    Serial.println(feederCalendar.getBool("feedOnThu") );
    Serial.print("Stored feedOnFriday: ");
    Serial.println(feederCalendar.getBool("feedOnFri") );
    Serial.print("Stored feedOnSaturday: ");
    Serial.println(feederCalendar.getBool("feedOnSat") );
    Serial.print("Stored feedOnSunday: ");
    Serial.println(feederCalendar.getBool("feedOnSun") );

    Serial.println("Current feeding calendar stored");

    feederCalendar.end();
}

char* Feeder::getFirstFeedDateTime()
{
    char* str = (char*)malloc(10 * sizeof(char));
    strcpy(str, firstFeedDateTime);
    return str;
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

        if (feedCalendarEnabled) {
            struct tm timeinfo;
            if(!getLocalTime(&timeinfo, 10)){
                Serial.println("Failed to obtain time");
            }
            switch (timeinfo.tm_wday) {
                case 0:
                    if (!feedOnSunday) return;
                    break;
                case 1:
                    if (!feedOnMonday) return;
                    break;
                case 2:
                    if (!feedOnTuesday) return;
                    break;
                case 3:
                    if (!feedOnWednesday) return;
                    break;
                case 4:
                    if (!feedOnThursday) return;
                    break;
                case 5:
                    if (!feedOnFriday) return;
                    break;
                case 6:
                    if (!feedOnSaturday) return;
                    break;
                default:
                    break;
            }
        }
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

