/*
 *	MAIN POT WEIGHT READ + CONTROL
 *	
 *	57 	weights
 *	 4 	16ch multiplexer 					all on same data pins, individual toggle
 *	29	2ch motor controllers
 *	 4	8ch shift register 				chained
 *  
 *
 *  Note: H-Bridge Motor pins are mirrored!
*/

#define TOLERANCE 40    // threshold value for nn to pot matching
#define AVERAGE 3
float *weights[57];     // arr of pointers to all nn_weights (incl. biases)
float weights_avg[57][AVERAGE];  // used to store values for weight rolling average
int average_ticker = 0;

// MULTIPLEXER
#define MULTIS 1				// number of connected multiplexers
int controlPins[] = {33, 34, 35, 36};
int MULTI_Z = A18;
int MULTI_EN[1] = {38}; // pins for MUX on/off toggles (4x)

// SHIFT REGISTER
#define SHIFTS 15       // number of chained shift REGISTERS
#define SR_LATCH_PIN 31 // Pin connected to ST_CP of 74HC595
#define SR_CLOCK_PIN 30 // Pin connected to SH_CP of 74HC595
#define SR_DATA_PIN  32 // Pin connected to DS of 74HC595
byte srs[SHIFTS];           // arr of bytes for (57*2)/8 = 15 chained shift registers
  

void weights_setup() {
	// set multiplexer control pins
  for(int i=0; i<4; i++){
    pinMode(controlPins[i], OUTPUT);
    digitalWrite(controlPins[i], LOW);
  }
  // disable all mux boards
  for(int i=0; i<MULTIS; i++){
    pinMode(MULTI_EN[i], OUTPUT);
    digitalWrite(MULTI_EN[i], HIGH); // high == off
  }

  // set shift register pins
  pinMode(SR_LATCH_PIN, OUTPUT);
  pinMode(SR_CLOCK_PIN, OUTPUT);
  pinMode(SR_DATA_PIN, OUTPUT);
  // set bytes for shift registers: all motors off.
  for(int i=0; i<SHIFTS; i++){
    srs[i] = 0b00000000;
  }

  // set weight avg storage to zero
  for(int i=0; i<57; i++){
    for(int j=0; j<AVERAGE; j++){
      weights_avg[i][j] = 0;
    }
  }

  // assign weight pointers to weights arr
  // not sure if there's a better way than to do it manually?

  // conv 0
  weights[0] = &conv_weights[0][0][0];
  weights[1] = &conv_weights[0][0][1];
  weights[2] = &conv_weights[0][0][2];
  weights[3] = &conv_weights[0][1][0];
  weights[4] = &conv_weights[0][1][1];
  weights[5] = &conv_weights[0][1][2];
  weights[6] = &conv_weights[0][2][0];
  weights[7] = &conv_weights[0][2][1];
  weights[8] = &conv_weights[0][2][2];

}


void weights_init() {
  while(weights_update() > TOLERANCE) {
    // weights_update() takes care of it already ...
  }
}


int weights_update() {

  int n_weights = 4; // temporary
  int total_diff = 0;

  // go through all connected weights -> currently only 9 (conv 0)!
  for(int weight=0; weight<n_weights; weight++){
    // read nn weight and compare to pot reading
    int diff = read_weight(weight, true) - map(*weights[weight], -1, 1, 0, 1023);
    // add difference to total
    total_diff += abs(diff);
    // determine shift register
    int sr = SHIFTS -1 - floor(weight / 4);
    // determine pos on register
    int pos = (weight % 4) * 2;
    // 0/0 turn off
    if(abs(diff) < TOLERANCE) {
      bitWrite(srs[sr], pos,     0);
      bitWrite(srs[sr], pos + 1, 0);
    // 1/0 turn left
    } else if(diff > 0) {
      bitWrite(srs[sr], pos,     1);
      bitWrite(srs[sr], pos + 1, 0);
    // 0/1 turn right
    } else if(diff < 0) {
      bitWrite(srs[sr], pos,     0);   
      bitWrite(srs[sr], pos + 1, 1);
    }
  }

  // write shift registers
  digitalWrite(SR_LATCH_PIN, LOW);
  for(int i=0; i<SHIFTS; i++){
    shiftOut(SR_DATA_PIN, SR_CLOCK_PIN, MSBFIRST, srs[i]);
  }
  digitalWrite(SR_LATCH_PIN, HIGH);

  // return avg. diff for all weights
  return total_diff / n_weights;
}



/* MULTIPLEXER: reading pots / weights -------------------------- */

// read weight without averaging
int read_weight(int channel) {
  return read_weight(channel, false);
}

// read weight value from multiplexer
int read_weight(int weight, bool avg) {
  // choose mux and enable
  int selected_mux = floor(weight / 16);
  digitalWrite(MULTI_EN[selected_mux], LOW);
  // match total weight to board channels
  int selected_channel = weight - selected_mux * 16;
  // set control to channel
  for(int i=0; i<4; i++){
    digitalWrite(controlPins[i], bitRead(selected_channel, i));
  }
  int reading = analogRead(MULTI_Z);
  // turn off mux
  digitalWrite(MULTI_EN[selected_mux], HIGH);
  //return the value 
  if(!avg) {
    return reading;
  } else {
    // store current reading
    weights_avg[weight][average_ticker] = reading;
    // average all readings
    float averaged = 0;
    for(int i=0; i<AVERAGE; i++){
      averaged += weights_avg[weight][i];
    }
    // update ticker and reset if necessary
    average_ticker++;
    if(average_ticker >= AVERAGE) {
      average_ticker = 0;
    }
    return averaged / AVERAGE;
  }
}

/* SHIFT REGISTERS: motor control  ----------------------------- */

// toggle motor control to update weight pot
// action:
// 0 off
// 1 turn left = R goes up
// 2 turn right = R goes down
void moveWeight(int weight, int action) {

  // choose H-Bridge
  // int bridge_id = floor(weight / 2);
  
  // Write shift register:
  // 2 bytes per motor
  // 10 turn left (?)
  // 01 turn right (?)
  // 00 stop
  byte bitsToSend = 0b00000000;
  // currently static, todo: address correct H-Bridge for multiple bridges
  // turn on H-Bridge 1

  if(action == 1) {
    bitWrite(bitsToSend, weight, HIGH);     // 1 0
  } else if(action == 2) {
    bitWrite(bitsToSend, weight+1, HIGH);   // 0 1
  }

  digitalWrite(SR_LATCH_PIN, LOW);
  shiftOut(SR_DATA_PIN, SR_CLOCK_PIN, MSBFIRST, bitsToSend);
  digitalWrite(SR_LATCH_PIN, HIGH);
}


/* HELPER FUNCTIONS ------------------------------------------- */

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



