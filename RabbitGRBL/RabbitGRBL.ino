

/*
  Builds Correctly with v2.0.0 ESP32 Board Package for Arduino

  For help use the WikiPage 
  https://github.com/SourceRabbit/RabbitGRBL/wiki/How-to-Compile-with-Arduino-IDE
*/

#include "src/Grbl.h"

void setup() {
  grbl_init();
}

void loop() {
  run_once();
}
