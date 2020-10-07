/*
 *	MAIN POT WEIGHT READ + CONTROL
 *	
 *	57 	weights
 *	 5 	16ch multiplexer 					all on same data pins, individual toggle
 *	29	2ch motor controllers
 *	 4	8ch shift register 				chained
 *  
 *
 *  Note: H-Bridge Motor pins are mirrored!
*/

#define N_WEIGHTS 40     // number of connected weights
#define TOLERANCE 30    // threshold value for nn to pot matching
#define AVERAGE 3       // default value for averaging

float min_map = 30.;        // minimum pot R value (will be updated in setup)
float max_map = 1020.;     // maximum pot R value (will be updated in setup)
  
float *weights[N_WEIGHTS];     // arr of pointers to all nn_weights (incl. biases)

// MULTIPLEXER
#define MULTIS 4				// number of connected multiplexers
int MULTI_EN[MULTIS] = {6,7,8,9}; // pins for MUX on/off toggles (5x) 6-10
int controlPins[] = {2, 3, 4, 5};
int MULTI_Z = A0;       // = PIN 14

// SHIFT REGISTER
#define SHIFTS 12       // number of chained shift REGISTERS
#define SR_LATCH_PIN 12 // Pin connected to ST_CP of 74HC595
#define SR_CLOCK_PIN 11 // Pin connected to SH_CP of 74HC595
#define SR_DATA_PIN  15 // Pin connected to DS of 74HC595
byte srs[SHIFTS];           // arr of bytes for (57*2)/8 = 15 chained shift registers


void weights_setup(bool calibrate) {
	// set multiplexer control pins
  for(int i=0; i<4; i++){
    pinMode(controlPins[i], OUTPUT);
    digitalWrite(controlPins[i], LOW);
  }
  // set mux input
  pinMode(MULTI_Z, INPUT);
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
  // set all shift registers
  weights_propagate();

  // calibrate
  if(calibrate) {
    weights_calibrate();
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
  weights[9] = &conv_bias[0];

  weights[10] = &conv_weights[1][0][0];
  weights[11] = &conv_weights[1][0][1];
  weights[12] = &conv_weights[1][0][2];
  weights[13] = &conv_weights[1][1][0];
  weights[14] = &conv_weights[1][1][1];
  weights[15] = &conv_weights[1][1][2];
  weights[16] = &conv_weights[1][2][0];
  weights[17] = &conv_weights[1][2][1];
  weights[18] = &conv_weights[1][2][2];
  weights[19] = &conv_bias[1];
}


void weights_init() {
  while(weights_update() > TOLERANCE) {
    // weights_update() takes care of it already ...
  }
}


int weights_update() {
  int total_diff = 0;
  // go through all connected weights -> currently only 9 (conv 0)!
  for(int weight=0; weight<N_WEIGHTS; weight++){
    // read nn weight and compare to pot reading
    int diff = weight_read(weight) - map_float(*weights[weight], -2, 2, min_map, max_map);
    // add difference to total
    total_diff += abs(diff);
    int action = 0;
    // 0/0 turn off
    if(abs(diff) < TOLERANCE) {
      action = 0;
    // 1/0 turn left
    } else if(diff > 0) {
      action = -1;
    // 0/1 turn right
    } else if(diff < 0) {
      action = 1;
    }
    weight_move(weight, action);
  }

  // pushes changes out to shift registers
  weights_propagate();

  // return avg. diff for all weights
  return total_diff / N_WEIGHTS;
}


void weights_propagate() {
  // write shift registers
  digitalWrite(SR_LATCH_PIN, LOW);
  for(int i=0; i<SHIFTS; i++){
    shiftOut(SR_DATA_PIN, SR_CLOCK_PIN, MSBFIRST, srs[i]);
  }
  digitalWrite(SR_LATCH_PIN, HIGH);
}


/* MULTIPLEXER: reading pots / weights -------------------------- */

// read weight with default avg
int weight_read(int channel) {
  return weight_read(channel, AVERAGE);
}

// read weight value from multiplexer
int weight_read(int weight, int avg) {
  // choose mux and enable
  int selected_mux = floor(weight / 10); // MUX has 16 capacity, but I only connect 10 per mux for the first 4 of them
  // match total weight to board channels
  int selected_channel = weight - selected_mux * 10;
  // set control to channel
  digitalWrite(MULTI_EN[selected_mux], LOW);
  //delay(1);
  for(int i=0; i<4; i++){
    digitalWrite(controlPins[i], bitRead(selected_channel, i));
  }
  //delay(1);                        // improves reading accuracy
  analogRead(MULTI_Z); // improves reading accuracy
  int reading = analogRead(MULTI_Z);
  // turn off mux
  digitalWrite(MULTI_EN[selected_mux], HIGH);
  //return the value 
  if(avg < 2) {
    return reading;
  } else {
    for(int i=0; i<avg-1; i++){
      reading += analogRead(MULTI_Z);
    }
    return reading / avg;
  }
}

float weight_read_multiple(int start, int weights) {
  float avg = 0;
  int count = 0;
  for(int i=start; i<start+weights; i++){
    // Function currently ignores bias weights, 
    // as those aren't currently connected ...
    if((i+1)%10 != 0) {
      avg += weight_read(i);
      count ++;
    }
  }
  return avg / count;
}

/* SHIFT REGISTERS: motor control  ----------------------------- */

// toggle motor control to update weight pot
// action:
//  0 off
// -1 turn left = R goes up
//  1 turn right = R goes down
void weight_move(int weight, int action) {
  // determine board (i.e. PCB) with 3 shift registers each ... currently (!)
  int board = floor(weight / 10);
  // determine shift register (3 per board)
  int sr = SHIFTS -1 - board * 3 - floor((weight-board*10) / 4);
  // old: int sr = SHIFTS -1 - floor(weight / 4);
  // determine pos on register
  int pos = ((weight-board*10) % 4) * 2;
  if(action == 0) {
    bitWrite(srs[sr], pos,     0);
    bitWrite(srs[sr], pos + 1, 0);
  }
  if(action == -1) {
    bitWrite(srs[sr], pos,     1);
    bitWrite(srs[sr], pos + 1, 0);
  }
  if(action == 1) {
    bitWrite(srs[sr], pos,     0);   
    bitWrite(srs[sr], pos + 1, 1);
  }
}


void weights_calibrate() {
  Serial.println("CALIBRATING WEIGHTS");
  // measure weight min max avg for map()
  // 1. Turn all to left
  Serial.println("Moving all weights to full left");
  int prev_weight_avg = weight_read_multiple(0, N_WEIGHTS);
  for(int i=0; i<N_WEIGHTS; i++) {
    weight_move(i, -1);
  }
  weights_propagate();
  delay(1000);
  int weight_avg = 0;
  while( abs(weight_avg - prev_weight_avg) > TOLERANCE ) {
    prev_weight_avg = weight_avg;
    delay(1000);
    weight_avg = weight_read_multiple(0, N_WEIGHTS);
  }
  delay(1000);
  for(int i=0; i<N_WEIGHTS; i++) {
    weight_move(i, 0);
  }
  weights_propagate();
  delay(1000);
  min_map = weight_read_multiple(0, N_WEIGHTS);
  Serial.print("Register min_map = ");
  Serial.println(min_map);
  // 2. Turn all to right
  Serial.println("Moving all weights to full right");
  prev_weight_avg = weight_read_multiple(0, N_WEIGHTS);
  for(int i=0; i<N_WEIGHTS; i++) {
    weight_move(i, 1);
  }
  weights_propagate();
  delay(1000);
  weight_avg = 1023;
  while( abs(weight_avg - prev_weight_avg) > TOLERANCE ) {
    prev_weight_avg = weight_avg;
    delay(2000);
    weight_avg = weight_read_multiple(0, N_WEIGHTS);
  }
  delay(1000);
  for(int i=0; i<N_WEIGHTS; i++) {
    weight_move(i, 0);
  }
  weights_propagate();
  delay(1000);
  max_map = weight_read_multiple(0, N_WEIGHTS);
  Serial.print("Register max_map = ");
  Serial.println(max_map);

  for(int i=0; i<N_WEIGHTS; i++){
    weight_move(i, 0);
  }
  weights_propagate();
}


/* HELPER FUNCTIONS ------------------------------------------- */

float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}




void blinkShifts() {
  for(int i=0; i<SHIFTS; i++){
    srs[i] = 0b11111111;
  }
  // set all shift registers
  weights_propagate();
  delay(1000);
  for(int i=0; i<SHIFTS; i++){
    srs[i] = 0b00000000;
  }
  // set all shift registers
  weights_propagate();
  delay(1000);
}



