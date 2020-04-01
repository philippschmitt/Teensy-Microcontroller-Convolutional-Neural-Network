/* 
 *  CONTROL PIXEL DISPLAYS
 *  
 *  1x    8x8 input
 *  4x    6x6 activation
 *  4x    2x2 pool
 *
 *  Currently only one 8x8 pixel matrix connected
*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN     0
#define LED_COUNT   224 // total px count
#define BRIGHTNESS  50

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Display arr pointer -> updated with data in other vars
unsigned int display_seq[LED_COUNT];


void setup_display() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
}


void display_test() {
  // assign memory for a data sample
  float matrix[8][8] = {-1};
  // generate a sample and display
  int Y = generate_sample(matrix);
  Y--;  // this is just to remove the console warning of an unused Y
  display_8x8(matrix);
  delay(500);
}


// set the pointers for the arr
// theoretically then I can update the data without having reorder stuff
void display_8x8 (float (*data)[8]) {
  display_8x8(data, 0, 1);
}

void display_8x8 (float (*data)[8], int mapLow, int mapHigh) {
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
        display_seq[cursor] = map(data[y][x], mapLow, mapHigh, 0, 255);
      } else {
        display_seq[cursor] = map(data[y][size-1 -x], mapLow, mapHigh, 0, 255);
      }
      cursor++;
    }
  }
  update_display();
}


// for activation functions only right now
// will extend to be general display function for any display
void display (float (*data)[6], int mapLow, int mapHigh) {

  // will be extended. for now, simply pack in 8x8 matrix and display on input display
  float matrix[8][8];
  set_matrix(matrix, -1);
  for(int y=0; y<6; y++){
    for(int x=0; x<6; x++){
      matrix[y][x] = data[y][x];
    }
  }
  delay(1000);
  display_8x8(matrix, mapLow, mapHigh);
}


// for pool activation
void display(float (*data)[2][2], int mapLow, int mapHigh) {
  float matrix[8][8];
  set_matrix(matrix, mapLow);
  matrix[0][0] = data[0][0][0];
  matrix[0][1] = data[0][0][1];
  matrix[1][0] = data[0][1][0];
  matrix[1][1] = data[0][1][1];

  matrix[2][2] = data[1][0][0];
  matrix[2][3] = data[1][0][1];
  matrix[3][2] = data[1][1][0];
  matrix[3][3] = data[1][1][1];

  matrix[4][4] = data[2][0][0];
  matrix[4][5] = data[2][0][1];
  matrix[5][4] = data[2][1][0];
  matrix[5][5] = data[2][1][1];

  matrix[6][6] = data[2][0][0];
  matrix[6][7] = data[2][0][1];
  matrix[7][6] = data[2][1][0];
  matrix[7][7] = data[2][1][1];

  delay(1000);
  display_8x8(matrix, mapLow, mapHigh);
}


// set individual LEDs at starting point to end point
void update_display() {
  int start = 0;
  int length = 224;
  for(int i=0; i<length;i++) {
    strip.setPixelColor(i+start, strip.Color(
      0, 
      0, 
      0, 
      strip.gamma8(display_seq[i])
    ));
  }
  strip.show();
}
