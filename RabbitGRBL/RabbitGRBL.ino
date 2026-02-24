

/*
  Builds Correctly with v2.0.0 ESP32 Board Package for Arduino
*/

#include "src/Grbl.h"

void setup() {
  grbl_init();
}

void loop() {
  run_once();
}
