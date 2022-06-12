/* 
 *  CONTROL PIXEL DISPLAYS
 *  
 *  1x    8x8  input         ID: 0
 *  4x    6x6  activation    ID: 1-4
 *  4x    2x2  pool          ID: 5-8 
 *  1x    16x1 lin           ID: 9

*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN     1
#define LED_COUNT   240 // total px count
#define BRIGHTNESS  150

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Display arr pointer -> updated with data in other vars
float display_seq[LED_COUNT];


void display_setup() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (up to 255)
}


void display_test() {
  for(int i=0; i<4; i++){
    for(int i=0; i<LED_COUNT;i++) {
      strip.setPixelColor(i+0, strip.Color(
        0, 
        0, 
        0, 
        255
      ));
    }
    strip.show();
    delay(100);
    for(int i=0; i<LED_COUNT;i++) {
      strip.setPixelColor(i+0, strip.Color(
        0, 
        0, 
        0, 
        0
      ));
    }
    strip.show();
    delay(100);   
  }
}


void display (float (*data)[8], int displayID, int mapLow, int mapHigh, bool update) {
  if(displayID != 0) {
    Serial.println("Data/display mismatch");
  }
  // cursor is position in overall display arr (224 neopixels)
  int cursor = 0;
  int size = 8; // one dimension
  // serialize input matrix
  // line
  for (int y=0; y<size; y++) {
    // column
    for (int x=0; x<size; x++) {
      // go backwards for even-numbered lines (due to wiring)
      if(y%2 == 0) {
        display_seq[cursor] = map_float(data[y][x], mapLow, mapHigh, 0, 255);
      } else {
        display_seq[cursor] = map_float(data[y][size-1 -x], mapLow, mapHigh, 0, 255);
      }
      cursor++;
    }
  }
  if(update) {
    display_update();
  }
}


// for conv activations
void display (float (*data)[6], int displayID, int mapLow, int mapHigh, bool update) {
  int size = 6; // one dimension
  // cursor is position in overall display arr (224 neopixels)
  int cursor = 64 + (displayID-1) * size*size;
  // serialize input matrix
  // line
  for (int y=0; y<size; y++) {
    // column
    for (int x=0; x<size; x++) {
      // go backwards for even-numbered lines (due to wiring)
      if(y%2 == 0) {
        display_seq[cursor] = map_float(data[y][x], mapLow, mapHigh, 0, 255);
      } else {
        display_seq[cursor] = map_float(data[y][size-1 -x], mapLow, mapHigh, 0, 255);
      }
      cursor++;
    }
  }
  if(update) {
    display_update();
  }
}


// Display IDs go backwards for pool layers:
// P4 = 5
// P3 = 6
// P2 = 7
// P1 = 8

// for pool layer
void display(float (*data)[2], int displayID, int mapLow, int mapHigh, bool update) {
  int size = 2; // one dimension
  // cursor is position in overall display arr (224 neopixels)
  int cursor = 208 + (displayID-5) * size*size;
  // serialize input matrix
  // line
  for (int y=0; y<size; y++) {
    // column
    for (int x=0; x<size; x++) {
      // go backwards for even-numbered lines (due to wiring)
      if(y%2 == 0) {
        display_seq[cursor] = map_float(data[y][x], mapLow, mapHigh, 0, 255);
      } else {
        display_seq[cursor] = map_float(data[y][size-1 -x], mapLow, mapHigh, 0, 255);
      }
      cursor++;
    }

  }

  if(update) {
    display_update();
  }
}


// for lin layer
void display(float *data, int displayID, int mapLow, int mapHigh, bool update) {
  int size = 16; // input dimension
  // cursor is position in overall display arr (224 neopixels)
  int cursor = 224;
  for (int x=0; x<size; x++) {
    display_seq[cursor] = map_float(data[x], mapLow, mapHigh, 0, 255);
    cursor++;
  }
  if(update) {
    display_update();
  }
}


// set individual LEDs at starting point to end point
void display_update() {
  for(int i=0; i<LED_COUNT;i++) {
    strip.setPixelColor(i, strip.Color(
      0, 
      0, 
      0, 
      strip.gamma8(display_seq[i])
    ));
  }
  strip.show();
}


// blink teensy LED
void blink() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
}

// cycle through LEDs to see if everything works
void display_cycle() {
  int cursor = 0;
  for(int y=0; y<LED_COUNT; y++){
    for(int i=0; i<LED_COUNT; i++){
      strip.setPixelColor(i, strip.Color(
        0, 
        0, 
        0, 
        (y==cursor) ? 255 : 0
      ));
      strip.show();
      
      delay(50);   
    }
  }
}


// fade out all pixels
void display_fadeout(int speed) {
  int speed_coeff = 3;
  int delta = 1;

  while(delta > 0) {
    delta = 0;
    for(int i=0; i<LED_COUNT; i++){
      delta += display_seq[i];
      display_seq[i] = max(display_seq[i]-speed*speed_coeff, 0);
    }
    display_update();
  }

}

