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

	// generates rand inputs and displays
  // display_test();

  // create array for sample and fill with zeros
  //float sample[8][8];
  //set_matrix(sample, 0);
  // generate a random sample: fills passed matrix, returns Y
  //int Y = generate_sample(sample);

  // dev overwrite: fixed sample, control group in ipy notebook
  
  float sample[8][8] = {
    {0., 0., 0., 0., 0., 0., 0., 0.},
    {0., 0., 0., 0., 0., 0., 0., 0.},
    {0., 0., 0., 1., 1., 1., 0., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 1., 1., 0., 0.}
  };
  int Y = 0;
  
  // display sample on input screen
  display_8x8(sample);
  
  // predict sample
  // float y = nn_predict(sample, true);

  // train with current example
  nn_train(sample, Y);

  delay(2000);
}

void loop() {
}
