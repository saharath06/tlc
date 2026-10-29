#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

#define USB_VID 0x303a
#define USB_PID 0x1001
#define USB_as_HID 1

// Built-in WS2812 RGB Status LED on ESP32-S3-DevKitC-1
#define PIN_RGB_LED 48
static const uint8_t LED_BUILTIN = SOC_GPIO_PIN_COUNT + PIN_RGB_LED;
#define BUILTIN_LED LED_BUILTIN
#define RGB_BUILTIN LED_BUILTIN
#define RGB_BRIGHTNESS 64
#define HAS_RGB_LED 1
#define RGB_LED 48
#define LED_TYPE WS2812B
#define LED_ORDER GRB
#define LED_TYPE_IS_RGBW 0
#define LED_COUNT 1
#define LED_COLOR_STEP 15

// UART0 & GPS Serial Pins
static const uint8_t TX = 43;
static const uint8_t RX = 44;
#define SERIAL_TX 17
#define SERIAL_RX 18
#define GPS_SERIAL_TX 17
#define GPS_SERIAL_RX 18
#define BAD_TX 17
#define BAD_RX 18

// I2C Bus (PN532 NFC/RFID V3 & External I2C Sensors)
#define GROVE_SDA 8
#define GROVE_SCL 9
#define SYS_I2C_SDA 8
#define SYS_I2C_SCL 9
static const uint8_t SDA = 8;
static const uint8_t SCL = 9;

// Secondary RF SPI Bus (SPI_2: CC1101 Sub-GHz + nRF24L01+PA+LNA)
// Separated from TFT SPI_1 to prevent screen tearing during RF scanning
#define SPI_SCK_PIN 40
#define SPI_MOSI_PIN 41
#define SPI_MISO_PIN 42
#define SPI_SS_PIN 10

static const uint8_t SS = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 13;
static const uint8_t SCK = 12;

// CC1101 Sub-GHz Module
#define USE_CC1101_VIA_SPI 1
#define CC1101_SCK_PIN 40
#define CC1101_MOSI_PIN 41
#define CC1101_MISO_PIN 42
#define CC1101_SS_PIN 10
#define CC1101_GDO0_PIN 14
#define CC1101_GDO2_PIN -1

// nRF24L01+PA+LNA 2.4GHz Module
#define USE_NRF24_VIA_SPI 1
#define NRF24_SCK_PIN 40
#define NRF24_MOSI_PIN 41
#define NRF24_MISO_PIN 42
#define NRF24_CE_PIN 38
#define NRF24_SS_PIN 39

// SD Card on Display SPI_1 Bus
#define SDCARD_CS 21
#define SDCARD_SCK 12
#define SDCARD_MISO 13
#define SDCARD_MOSI 11

// 2.8" ILI9341 SPI Color Display + XPT2046 Resistive Touch
#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT (uint8_t)10
#define BACKLIGHT 45

#define USER_SETUP_LOADED 1
#define USE_HSPI_PORT 1
#define ILI9341_DRIVER 1
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 16
#define TFT_BL 45
#define TFT_BACKLIGHT_ON HIGH
#define SMOOTH_FONT 1
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000

// Touch Screen (XPT2046 SPI)
#define HAS_TOUCH 1
#define USE_TFT_eSPI_TOUCH 1
#define TOUCH_CS 7

// Physical 5-Way Navigation Buttons (Work simultaneously alongside Touch!)
#define HAS_5_BUTTONS 1
#define BTN_ALIAS "\"OK\""
#define SEL_BTN 0
#define UP_BTN 1
#define DW_BTN 3
#define L_BTN 6
#define R_BTN 46
#define BTN_ACT LOW

// Infrared TX & RX
#define TXLED 4
#define RXLED 5
#define LED_ON HIGH
#define LED_OFF LOW

// Battery Voltage Monitoring (ADC1 with 2x 100kOhm divider)
#define ANALOG_BAT_PIN 1

// Font sizes
#define FP 1
#define FM 2
#define FG 3

#endif /* Pins_Arduino_h */
