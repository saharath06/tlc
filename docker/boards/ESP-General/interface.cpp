#include "core/bus_HAL.h"
#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>
#include <Wire.h>
#include <globals.h>
#include <interface.h>

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   Initial hardware setup for Bruce-S3 Pro Touch (ESP32-S3 N16R8)
***************************************************************************************/
void _setup_gpio() {
    // 1. De-assert all SPI Chip Select lines first to prevent bus contention on boot
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);

    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);

    pinMode(CC1101_SS_PIN, OUTPUT);
    digitalWrite(CC1101_SS_PIN, HIGH);

    pinMode(NRF24_SS_PIN, OUTPUT);
    digitalWrite(NRF24_SS_PIN, HIGH);

    pinMode(NRF24_CE_PIN, OUTPUT);
    digitalWrite(NRF24_CE_PIN, LOW);

    // 2. Configure 5-Way Tactile Navigation Buttons (Active LOW with internal pull-up)
    pinMode(SEL_BTN, INPUT_PULLUP);
    pinMode(UP_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
    pinMode(L_BTN, INPUT_PULLUP);
    pinMode(R_BTN, INPUT_PULLUP);

    // 3. Initialize I2C Bus for PN532 NFC/RFID V3 & External I2C Modules
    setSysI2CBus(&Wire);
    Wire.setPins(SYS_I2C_SDA, SYS_I2C_SCL);
    Wire.begin(SYS_I2C_SDA, SYS_I2C_SCL);

    // 4. Set Default Hardware Modules in Bruce Configuration
    bruceConfig.colorInverted = 0;
    bruceConfigPins.rfModule = CC1101_SPI_MODULE;
    bruceConfigPins.rfidModule = PN532_I2C_MODULE;
    bruceConfigPins.irTx = TXLED;
    bruceConfigPins.irRx = RXLED;
    bruceConfigPins.gps_bus.rx = (gpio_num_t)GPS_SERIAL_RX;
    bruceConfigPins.gps_bus.tx = (gpio_num_t)GPS_SERIAL_TX;
    bruceConfigPins.gpsBaudrate = 9600;
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   Second stage GPIO & Touch Calibration setup after TFT & LittleFS init
***************************************************************************************/
void _post_setup_gpio() {
    // Enable TFT Backlight PWM
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 255);

#ifdef HAS_TOUCH
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    // Pre-calibrated XPT2046 touch matrix for 2.8" ILI9341 (320x240 Landscape)
    // Prevents infinite calibration hang on first boot or inside Wokwi Simulator!
    uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    if (LittleFS.exists("/calData")) {
        File f = LittleFS.open("/calData", "r");
        if (f) {
            for (int i = 0; i < 5; i++) {
                String line = f.readStringUntil('\n');
                if (line.length() > 0) calData[i] = (uint16_t)line.toInt();
            }
            f.close();
        }
    } else {
        File f = LittleFS.open("/calData", "w");
        if (f) {
            f.printf("%d\n%d\n%d\n%d\n%d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
            f.close();
        }
    }
    tft.setTouch(calData);
#endif
}

/***************************************************************************************
** Function name: getBattery()
** Location: display.cpp
** Description:   Reads LiPo battery percentage (0-100%) with EMA smoothing filter
***************************************************************************************/
int getBattery() {
    uint32_t mv = analogReadMilliVolts(ANALOG_BAT_PIN);
    // If running in Wokwi Simulator or powered via USB without battery divider wired, show 95%
    if (mv < 500) return 95;

    float battMv = mv * 2.0f; // 2x 100kOhm voltage divider
    int pct = (int)(((battMv - 3300.0f) / (4200.0f - 3300.0f)) * 100.0f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

/***************************************************************************************
** Function name: isCharging()
***************************************************************************************/
bool isCharging() { return false; }

/*********************************************************************
** Function: _setBrightness
** Location: settings.cpp
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        analogWrite(TFT_BL, 0);
    } else {
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
        analogWrite(TFT_BL, bl);
    }
}

/*********************************************************************
** Function: InputHandler
** Supports BOTH XPT2046 Touch Screen AND 5-Way Physical Buttons!
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    if (millis() - tm < 180 && !LongPress) return;

#ifdef HAS_TOUCH
    TouchPoint t;
    checkPowerSaveTime();
    bool touched = tft.getTouch(&t.x, &t.y);
    if (touched) {
        NextPress = false;
        PrevPress = false;
        UpPress = false;
        DownPress = false;
        SelPress = false;
        EscPress = false;
        AnyKeyPress = false;
        NextPagePress = false;
        PrevPagePress = false;
        touchPoint.pressed = false;

        if (bruceConfigPins.rotation == 3) {
            t.y = (tftHeight + 20) - t.y;
            t.x = tftWidth - t.x;
        } else if (bruceConfigPins.rotation == 0) {
            uint16_t tmp = t.x;
            t.x = map((tftHeight + 20) - t.y, 0, 320, 0, 240);
            t.y = map(tmp, 0, 240, 0, 320);
        } else if (bruceConfigPins.rotation == 2) {
            uint16_t tmp = t.x;
            t.x = map(t.y, 0, 320, 0, 240);
            t.y = map(tftWidth - tmp, 0, 240, 0, 320);
        }

        if (!wakeUpScreen()) AnyKeyPress = true;
        else return;

        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed = true;
        touchHeatMap(touchPoint);
        tm = millis();
        return;
    }
#endif

#ifdef HAS_5_BUTTONS
    bool _u = (digitalRead(UP_BTN) == BTN_ACT);
    bool _d = (digitalRead(DW_BTN) == BTN_ACT);
    bool _l = (digitalRead(L_BTN) == BTN_ACT);
    bool _r = (digitalRead(R_BTN) == BTN_ACT);
    bool _s = (digitalRead(SEL_BTN) == BTN_ACT);

    if (_s || _u || _d || _r || _l) {
        tm = millis();
        if (!wakeUpScreen()) AnyKeyPress = true;
        else return;
    }
    if (_l) PrevPress = true;
    if (_r) NextPress = true;
    if (_u) {
        UpPress = true;
        PrevPagePress = true;
    }
    if (_d) {
        DownPress = true;
        NextPagePress = true;
    }
    if (_s) SelPress = true;
    if (_l && _r) {
        EscPress = true;
        NextPress = false;
        PrevPress = false;
    }
#endif
}

/*********************************************************************
** Function: powerOff
** Deep sleep wakeup on SEL_BTN (GPIO 0)
**********************************************************************/
void powerOff() {
    digitalWrite(TFT_BL, LOW);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SEL_BTN, BTN_ACT);
    esp_deep_sleep_start();
}

/*********************************************************************
** Function: checkReboot
** Hold LEFT + RIGHT buttons for 3 seconds to power off safely
**********************************************************************/
void checkReboot() {
    if (digitalRead(L_BTN) == BTN_ACT && digitalRead(R_BTN) == BTN_ACT) {
        uint32_t time_count = millis();
        while (digitalRead(L_BTN) == BTN_ACT && digitalRead(R_BTN) == BTN_ACT) {
            if (millis() - time_count > 500) {
                int countDown = (millis() - time_count) / 1000 + 1;
                tft.setTextSize(1);
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                if (countDown < 4) {
                    tft.drawCentreString("PWR OFF IN " + String(countDown) + "/3", tftWidth / 2, 12, 1);
                } else {
                    tft.fillScreen(bruceConfig.bgColor);
                    while (digitalRead(L_BTN) == BTN_ACT || digitalRead(R_BTN) == BTN_ACT);
                    delay(200);
                    powerOff();
                }
                delay(10);
            }
        }
    }
}
