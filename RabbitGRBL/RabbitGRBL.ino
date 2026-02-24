/*
  Builds Correctly with v2.0.0 ESP32 Board Package for Arduino (ESP-IDF v4.4)

  For help use the WikiPage 
  https://github.com/SourceRabbit/RabbitGRBL/wiki/How-to-Compile-with-Arduino-IDE
*/

#include "src/Grbl.h"

// RabbitGRBL requires exactly ESP32 Arduino Core v2.0.0.
// Use the Arduino IDE Board Manager to install this specific version.
#if !defined(ESP_ARDUINO_VERSION)
#error "ESP32 Arduino Core version could not be detected. Please install ESP32 Arduino Core v2.0.0."
#elif ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#error "ESP32 Arduino Core v2.0.0 is required. Please install exactly v2.0.0 from the Arduino IDE Board Manager."
#endif

void setup() {
  grbl_init();
}

void loop() {
  run_once();
}