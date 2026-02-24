

/*
  Builds Correctly with v1.0.6 ESP32 Board Package for Arduino
*/

#include "src/Grbl.h"

void setup() {
  grbl_init();
}

void loop() {
  run_once();
}
