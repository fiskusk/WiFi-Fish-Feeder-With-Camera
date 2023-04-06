#include "esp_camera.h"
#include <WiFi.h>
#include "LedController.h"
#include "time.h"
#include <esp_task_wdt.h>
#include "Feeder.h"
#include "globals.h"
#include "Preferences.h"

// Board: ESP32 Wrover
// Partition scheme: Huge APP

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

//3 seconds WDT
#define WDT_TIMEOUT 15

Feeder feeder;
LedController led;
Preferences creditials;

void startCameraServer();

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 1 * 60 * 60;
const int   daylightOffset_sec = 0; // 3600;

command_t event = {None, 0};
bool wifiConnected = false;
unsigned long wifiReconnectionStartTime = 0;
const uint32_t wifiReconnectionDelay = 10000;

bool printLocalTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return false;
    }
    Serial.println(&timeinfo, "%A, %d.%B %Y %H:%M:%S");

    return true;
}

char* feederGetValues(char * p)
{
    p+=sprintf(p, "\"lightenabled\":%u,", feeder.getLightEnabled() );
    p+=sprintf(p, "\"autolight\":%u,", feeder.getAutomaticFeedingLight() );
    p+=sprintf(p, "\"feedingintervalenabled\":%u,", feeder.getFeedingIntervalEnabled() );
    p+=sprintf(p, "\"feedinginterval\":%u,", feeder.getFeedingInterval() );
    p+=sprintf(p, "\"feedingtime\":%u,", feeder.getFeedingTime() );
    p+=sprintf(p, "\"lastfeedingtime\":\"%s\",", feeder.getLastFeedingTime() );
    p+=sprintf(p, "\"feedercalendarenable\":%u,", feeder.getFeedingCalendarEnabled() );
    p+=sprintf(p, "\"feedonmon\":%u,", feeder.getFeedingOnMonday() );
    p+=sprintf(p, "\"feedontue\":%u,", feeder.getFeedingOnTuesday() );
    p+=sprintf(p, "\"feedonwed\":%u,", feeder.getFeedingOnWednesday() );
    p+=sprintf(p, "\"feedonthu\":%u,", feeder.getFeedingOnThursday() );
    p+=sprintf(p, "\"feedonfri\":%u,", feeder.getFeedingOnFriday() );
    p+=sprintf(p, "\"feedonsat\":%u,", feeder.getFeedingOnSaturday() );
    p+=sprintf(p, "\"feedonsun\":%u,", feeder.getFeedingOnSunday() );
    p+=sprintf(p, "\"firstfeed\":%s", "\"15:12:36\"" );
    return p;
}

void feederCheckCommand()
{
    switch (event.command) {
        case RunFishFeeder:
            feeder.startFeeding(millis());
            event.command = None;
            break;
        case FeedingIntervalEnabled:
            Serial.print("Feeding interval is: ");
            event.value ? Serial.println("ENABLED") : Serial.println("DISABLED");
            feeder.setFeedingIntervalEnabled(event.value);
            event.command = None;
            break;
        case SetFeedingInterval:
            Serial.print("Feeding interval changed to: ");
            Serial.println(event.value);
            feeder.setFeedInterval(event.value);
            event.command = None;
            break;
        case SetFeedingTime:
            Serial.print("Feeding time (seconds) changed to: ");
            Serial.println(event.value);
            feeder.setFeedingTime(event.value);
            event.command = None;
            break;
        case SetLight:
            Serial.print("Feeding light is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setLightEnabled(event.value);
            event.command = None;
            break;
        case SetAutomaticLight:
            Serial.print("Automatic light when feeding changed to: ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setAutomaticFeedingLight(event.value);
            event.command = None;
            break;
        case SaveFeederSettings:
            feeder.saveDefaults();
            event.command = None;
            break;
        case SaveFeederCalendar:
            feeder.saveFeederCalendar();
            event.command = None;
            break;
        case SetFeederCalendarEnabled:
            Serial.print("Feeding Calendar is ");
            event.value ? Serial.println("ENABLED") : Serial.println("DISABLED");
            feeder.setFeedingCalendarEnabled(event.value);
            event.command = None;
            break;
        case SetFeedOnMonday:
            Serial.print("Feeding on Monday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnMonday(event.value);
            event.command = None;
            break;
        case SetFeedOnTuesday:
            Serial.print("Feeding on Thuesday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnTuesday(event.value);
            event.command = None;
            break;
        case SetFeedOnWednesday:
            Serial.print("Feeding on Wednesday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnWednesday(event.value);
            event.command = None;
            break;
        case SetFeedOnThursday:
            Serial.print("Feeding on Thursday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnThursday(event.value);
            event.command = None;
            break;
        case SetFeedOnFriday:
            Serial.print("Feeding on Friday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnFriday(event.value);
            event.command = None;
            break;
        case SetFeedOnSaturday:
            Serial.print("Feeding on Saturday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnSaturday(event.value);
            event.command = None;
            break;
        case SetFeedOnSunday:
            Serial.print("Feeding on Sunday is ");
            event.value ? Serial.println("ON") : Serial.println("OFF");
            feeder.setFeedingOnSunday(event.value);
            event.command = None;
            break;
        default:
            led.blink(50, 200, 50);
            event.command = None;
            break;
    }
    led.blink(500);
}

void serialInit()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    Serial.println();
    Serial.println("========================================");
    Serial.println("========================================");
}

void cameraInit()
{
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    if(psramFound()){
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    #if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    #endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t * s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    //s->set_framesize(s, FRAMESIZE_SXGA);

    #if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    #endif
}

void wifiCreditialsInit()
{
    creditials.begin("creditials", false);

    creditials.getString("ssid", "**EMPTY**");
    creditials.getString("password", "**EMPTY**");

    Serial.print("Stored ssid: ");
    Serial.println(creditials.getString("ssid") );
    //Serial.print("Stored password: ");
    //Serial.println(creditials.getString("password") );

    creditials.end();
}

bool wifiInit(unsigned long connectingStartingTime, uint32_t timeoutMs = 10000)
{
    creditials.begin("creditials", true);
    
    WiFi.begin(const_cast<char*>(creditials.getString("ssid").c_str()), 
               const_cast<char*>(creditials.getString("password").c_str()));
    creditials.end();
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED && millis() < connectingStartingTime + timeoutMs) {
        esp_task_wdt_reset();
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println();
        return true;
    }
    WiFi.disconnect();
    Serial.println("");
    Serial.println("WiFi connection timeout");
    Serial.println();
    return false;
}

void timeInit(int maxTryNr = 5)
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();
    for (int tryNr = 0; tryNr < maxTryNr; tryNr++) {
        if (printLocalTime());
            return;
    }
    Serial.println("The maximum number of attempts has been reached");
    struct tm timeinfo;
    Serial.println(&timeinfo, "%A, %d.%B %Y %H:%M:%S");
}

void commandNewCreditials()
{
    String received;
    creditials.begin("creditials", false);

    Serial.println("Enter new WiFi SSID:");
    while (Serial.available() == false) { esp_task_wdt_reset();}
    received = Serial.readString();
    creditials.putString("ssid", received);

    Serial.print("Enter new WiFi Password for \"");
    Serial.print(creditials.getString("ssid"));
    Serial.println("\":");
    while (Serial.available() == false) { esp_task_wdt_reset();}
    received = Serial.readString();
    creditials.putString("password", received);

    creditials.end();

    Serial.println("New Creditials Saved. Reboot now...");
    ESP.restart();
}

void scanWifi()
{
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    if (n == WIFI_SCAN_FAILED) {
        Serial.println("Scan failed");
        return;
    }
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(10);
        }
    }
}

void unknownCommand(String received)
{
    Serial.print("Received command \"");
    Serial.print(received);
    Serial.println("\" unknown!");
}

void checkSerialCommand(String received)
{
    if (received == "W") {
        Serial.println("Press \"N\" for enter new creditials; \"S\" for scan WiFi Networks; \"R\" for reboot");
        while (Serial.available() == false) { esp_task_wdt_reset();}
        received = Serial.readString();
        if (received == "N")
            commandNewCreditials();
        else if (received == "S")
            scanWifi();
        else if (received == "R")
            ESP.restart();
        else 
            unknownCommand(received);
    } else {
        unknownCommand(received);
    }
}

void setup() {
    // Initialize status LED
    led.init(LED1_NUM, true);

    // Indicate that MCU is initializing
    led.on();

    // initialize serial port for debug
    serialInit();

    // initialize onboard camera
    cameraInit();

    // initialize WiFi connection
    wifiCreditialsInit();
    wifiConnected = wifiInit(millis(), 15000);

    if (wifiConnected) {
        // Init and get the time
        timeInit();
        // start camera server
        startCameraServer();
    }

    // Initialize feeder
    feeder.init();

    // configure ESP Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    // Print welcome info
    if (wifiConnected) {
        Serial.print("Feeder Ready! Use 'http://");
        Serial.print(WiFi.localIP());
        Serial.println("' to connect");
        led.repeatBlink(50, 1500);
    } else {
        Serial.println("WiFi not connected. For configure WiFi connection enter \"W\"");
        led.repeatBlink(100, 100);
    }
}

void loop() {
    unsigned long time = millis();

    led.process(time);
    feeder.process(time);
    esp_task_wdt_reset();

    if (Serial.available() == true)
        checkSerialCommand(Serial.readString());
}
