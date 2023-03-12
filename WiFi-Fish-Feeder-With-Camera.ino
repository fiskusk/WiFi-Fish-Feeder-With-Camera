#include "esp_camera.h"
#include <WiFi.h>
#include "LedController.h"
#include "time.h"
#include <esp_task_wdt.h>

// Board: ESP32 Wrover
// Partition scheme: Huge APP

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

//3 seconds WDT
#define WDT_TIMEOUT 3

#include "camera_pins.h"

const char* ssid = "*** your ssid here ***";
const char* password = "*** your wi-fi password here ***";

LedController led;

void startCameraServer();
void checkFeederTimer(unsigned long time);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 1 * 60 * 60;
const int   daylightOffset_sec = 0; // 3600;

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

void setup() {
    // Initialize status LED
    led.init(LED1_NUM, true);

    // Indicate that MCU is initializing
    led.on();
    
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

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

    pinMode(4, OUTPUT);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();
    while (!printLocalTime());
    
    startCameraServer();

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

    checkFeederTimer(time);
    led.process(time);
    delay(10);
    esp_task_wdt_reset();
}
