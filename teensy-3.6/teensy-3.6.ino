/*
 *  PHYSICAL NEURAL NETWORK
 *  
 *  Note: Display functions currently drive one 8x8 neopixel matrix
 *
*/

void setup() {
  Serial.begin(9600);
  // setup neopixels
  setup_display();
  // initialize the neural net
  nn_setup();
}

void loop() {
	// generates rand inputs and displays
  // display_test();

  // create array for sample and fill with zeros
  float sample[8][8];
  set_matrix(sample, 0);
  // generate a random sample
  generate_sample(sample);
  // display sample on input screen
  display_8x8(sample);
  // predict sample
  nn_predict(sample, true);

  delay(2000);
}
