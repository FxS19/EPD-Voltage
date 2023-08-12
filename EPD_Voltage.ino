
// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
#define HIGH_RESISTOR 100000
#define LOW_RESISTOR 100000
#define VOLT_PIN 35
#define VOLT_OFFSET 0.08
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
#define TIME_TO_SLEEP 60 /* Time ESP32 will go to sleep (in seconds) */

// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
// #define LILYGO_T5_V102
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY_102      //Depend  https://github.com/adafruit/Adafruit_NeoPixel
// #define LILYGO_EPD_DISPLAY_154

#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>
#include <GxGDEW0213M21/GxGDEW0213M21.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

RTC_DATA_ATTR int bootCount = 0;
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, /*RST=*/ EPD_RSET, /*BUSY=*/ EPD_BUSY); // default selection of (9), 7

void setup()
{
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();
    display.eraseDisplay();
    // comment out next line to have no or minimal Adafruit_GFX code
    ++bootCount;
    display.drawPaged(drawHelloWorld); // version for AVR using paged drawing, works also on other processors
}

void drawHelloWorld()
{
    display.setTextColor(GxEPD_BLACK);
    Serial.println("Boot number: " + String(bootCount));
    print_wakeup_reason();
    double voltage = volt_read();
    display.print(voltage);
    display.print(" V (");
    display.print(((voltage - 3.0 )/1.2)*100);
    display.println("%)");
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : display.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : display.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : display.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : display.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : display.println("Wakeup caused by ULP program"); break;
    default : display.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

double rawVoltage() {
  // read analog and make it more linear
  double reading = analogRead(VOLT_PIN);
  for (int i = 0; i < 10; i++) {
    reading = (analogRead(VOLT_PIN) + reading * 9) / 10;
  }
  if (reading < 1 || reading > 4095) return 0;
  return -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
}

double convertVoltage(double volt) {
  return (HIGH_RESISTOR + LOW_RESISTOR) / LOW_RESISTOR  * volt + VOLT_OFFSET;
}


double volt_read(){
  return convertVoltage( rawVoltage());
}

void loop() {
  display.powerDown();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
};
