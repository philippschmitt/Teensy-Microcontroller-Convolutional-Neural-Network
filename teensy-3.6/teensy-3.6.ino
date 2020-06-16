/*
 *  PHYSICAL NEURAL NETWORK
 *  
 *  Note: Display functions currently drive one 8x8 neopixel matrix
 *
*/

#define EPOCHS          20000       // def: 20000
#define L_RATE          0.001
#define INPUT_SIZE      8
#define CONV_DEPTH      4
#define KERNEL_SIZE     3
#define CONV_SIZE       6       // divide: INPUT_SIZE / KERNEL_SIZE
#define POOLING_KERNEL  3
#define POOL_SIZE       2       // divide: CONV_SIZE / POOLING_KERNEL

// count epochs
int epoch = 0;


void setup() {
  Serial.begin(9600);
  // setup neopixels
  setup_display();
	// generates rand inputs and displays
  // display_test();
  
  // initialize the neural net
  nn_setup();

  // setup phyiscal weights
  weights_setup();

  // move weights to match nn state
  Serial.println("set weights to initial neural net state");
  weights_init();

  Serial.println("setup complete");
  Serial.println("\n");
}

void loop() {

  // create array for sample and fill with zeros
  float sample[8][8];
  set_matrix(sample, 0);
  // generate a random sample: fills passed matrix, returns Y
  int Y = generate_sample(sample);

  // display sample on input screen
  // display_8x8(sample);
  
  // predict sample
  // float y = nn_predict(sample, true);

  // train with current sample
  if(epoch < EPOCHS) {
    float loss = nn_train(sample, Y, false);
    epoch++;
    // Logging
    if(epoch % 50 == 0) {
      Serial.print(epoch);
      Serial.print("/");
      Serial.print(EPOCHS);
      Serial.print(" ");
      Serial.println(loss);

      // only move weights every couple of epochs
      int delta = weights_update();
    }
  }

}
