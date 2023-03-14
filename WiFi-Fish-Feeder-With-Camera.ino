#include "esp_camera.h"
#include <WiFi.h>
#include "LedController.h"
#include "time.h"
#include <esp_task_wdt.h>
#include <Preferences.h>
#include "Feeder.h"
#include "globals.h"

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
#define WDT_TIMEOUT 3

Feeder feeder;
LedController led;
Preferences defaults;

const char* ssid = "*** your ssid here ***";
const char* password = "*** your wi-fi password here ***";

void startCameraServer();

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 1 * 60 * 60;
const int   daylightOffset_sec = 0; // 3600;

unsigned long previousMillis = 0;   // ms
bool feedEnable = false;
unsigned long feedStartingTime;
unsigned long feedinterval;     // s
unsigned int feedingtime;       // s
bool feedingintervalenabled;
bool lightEnabled;
command_t event = {None, 0};

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

char* getValues(char * p)
{
    p+=sprintf(p, "\"lightenabled\":%u,", lightEnabled);
    p+=sprintf(p, "\"autolight\":%u,", feeder.getAutomaticFeedingLight());
    p+=sprintf(p, "\"feedingintervalenabled\":%u,", feedingintervalenabled);
    p+=sprintf(p, "\"feedinginterval\":%u,", feedinterval);
    p+=sprintf(p, "\"feedingtime\":%u", feedingtime);
    return p;
}

void checkCommand()
{
    defaults.begin("my-app", false);
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
            feeder.setFeedInterval(event.value*60UL*60UL*1000UL);
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
            defaults.putBool("pok", event.value);
            feeder.setAutomaticFeedingLight(event.value);
            event.command = None;
            break;
        default:
            led.blink(50, 200, 50);
            event.command = None;
            break;
    }
    led.blink(500);
    defaults.end();
}

void serialInit()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
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

void wifiInit()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}

void timeInit()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();
    while (!printLocalTime());
}

void setup() {
    // Initialize status LED
    led.init(LED1_NUM, true);

    // Indicate that MCU is initializing
    led.on();

    // initialize serial port for debug
    serialInit();

    // Open Preferences with my-app namespace. Each application module, library, etc
    // has to use a namespace name to prevent key name collisions. We will open storage in
    // RW-mode (second parameter has to be false).
    // Note: Namespace name is limited to 15 chars.
    defaults.begin("my-app", false);
    feedinterval = defaults.getULong("feedinterval", 12UL*60UL*60UL*1000UL);
    feedingtime = defaults.getUInt("feedingtime", 2);
    feedingintervalenabled = defaults.getBool("feedingintervalenabled", true);
    lightEnabled = defaults.getBool("lightEnabled", false);
    bool automaticFeedingLight = defaults.getBool("pok", true);

    Serial.print("Stored value is: ");
    defaults.getBool("pok") ? Serial.println("ON") : Serial.println("OFF");
    defaults.end();

    // initialize onboard camera
    cameraInit();

    // initialize WiFi connection
    wifiInit();

    // Init and get the time
    timeInit();
    
    // start camera server
    startCameraServer();

    // Initialize feeder
    feeder.init(automaticFeedingLight);
/*
    Serial.printf("Total heap: %d B\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d B\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d B\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d B\n", ESP.getFreePsram());
*/
    // Print welcome info
    Serial.print("Feeder Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");

    // configure ESP Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    led.repeatBlink(50, 1500);
}

void loop() {
    unsigned long time = millis();

    led.process(time);
    feeder.process(time);
    delay(10);
    esp_task_wdt_reset();
}
