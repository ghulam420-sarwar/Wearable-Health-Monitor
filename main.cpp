/**
 * Wearable Health Monitoring Device
 * ---------------------------------
 * Target MCU : ESP32-C3 Mini
 * Sensors    : MAX30102 (HR/SpO2, I2C), MLX90614 (IR body temp, I2C)
 * Output     : SSD1306 OLED, vibration motor alert
 * Author     : Ghulam Sarwar
 *
 * Reads heart rate (BPM) from MAX30102 using peak detection on the
 * filtered IR channel, reads non-contact body temperature from
 * MLX90614, displays both on the OLED, and triggers a vibration
 * motor alert if HR < 40 or HR > 160.
 *
 * MIT License
 */

#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN     8
#define SCL_PIN     9
#define MOTOR_PIN   6
#define BTN_PIN     5

MAX30105            particle;
Adafruit_MLX90614   mlx;
Adafruit_SSD1306    oled(128, 64, &Wire, -1);

constexpr uint8_t RATE_SIZE = 8;
uint8_t rates[RATE_SIZE];
uint8_t rateIdx = 0;
uint32_t lastBeatMs = 0;
float beatAvg = 0;
float bodyTempC = 0;

bool alarm(float bpm) {
    return (bpm > 0 && (bpm < 40 || bpm > 160));
}

void drawUi(float bpm, float temp, bool alertOn) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);

    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print(F("HEALTH MONITOR"));
    oled.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    oled.setTextSize(2);
    oled.setCursor(0, 16);
    if (bpm > 0) oled.printf("%3.0f BPM", bpm);
    else         oled.print(F(" -- BPM"));

    oled.setCursor(0, 38);
    oled.printf("%4.1f C", temp);

    oled.setTextSize(1);
    oled.setCursor(0, 56);
    oled.print(alertOn ? F("!! ALERT !!") : F("Status: OK"));
    oled.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!particle.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30102 not found");
        while (1) delay(1000);
    }
    particle.setup();                  // default config
    particle.setPulseAmplitudeRed(0x1F);
    particle.setPulseAmplitudeGreen(0);

    if (!mlx.begin()) Serial.println("MLX90614 not found");
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        Serial.println("OLED not found");

    drawUi(0, 0, false);
}

void loop() {
    long ir = particle.getIR();

    if (checkForBeat(ir)) {
        uint32_t now = millis();
        uint32_t delta = now - lastBeatMs;
        lastBeatMs = now;

        float bpm = 60000.0f / delta;
        if (bpm > 20 && bpm < 255) {
            rates[rateIdx++ % RATE_SIZE] = (uint8_t)bpm;
            uint16_t sum = 0;
            for (uint8_t i = 0; i < RATE_SIZE; ++i) sum += rates[i];
            beatAvg = (float)sum / RATE_SIZE;
        }
    }

    static uint32_t tTemp = 0;
    if (millis() - tTemp >= 1000) {
        tTemp = millis();
        bodyTempC = mlx.readObjectTempC();

        bool alert = alarm(beatAvg);
        digitalWrite(MOTOR_PIN, alert ? HIGH : LOW);
        drawUi(beatAvg, bodyTempC, alert);

        Serial.printf("BPM=%.0f  Tobj=%.2fC  Tamb=%.2fC  alert=%d\n",
                      beatAvg, bodyTempC, mlx.readAmbientTempC(), alert);
    }
}
