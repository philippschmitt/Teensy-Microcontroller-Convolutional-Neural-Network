/*
 *  PHYSICAL NEURAL NETWORK
 *  
 *  Note: Display functions currently drive one 8x8 neopixel matrix
 *
*/

#define EPOCHS          10000       // def: 20000
#define L_RATE          0.1       // def: 0.001
#define INPUT_SIZE      8
#define CONV_DEPTH      4
#define KERNEL_SIZE     3
#define CONV_SIZE       6       // divide: INPUT_SIZE / KERNEL_SIZE
#define POOLING_KERNEL  3
#define POOL_SIZE       2       // divide: CONV_SIZE / POOLING_KERNEL

// count epochs
unsigned int epoch = 0;

unsigned long time = 0;
unsigned long startTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("hello!");
  // blink to signal
  blink();
  // setup neopixels
  display_setup();
  
  // initialize the neural net
  nn_setup();

  // setup phyiscal weights
  // set calibrate to false for faster boot
  weights_setup(false);

  // move weights to match nn state
  Serial.println("set weights to initial neural net state");
  weights_init();

  Serial.println("setup complete");
  Serial.println("\n");

  blink(); // loop is beginning

  startTime = millis();
}


void loop() {

  if(millis()-startTime > 500 ) {
    // reset timer
    startTime = millis();

    // create array for sample and fill with zeros
    float sample[8][8];
    set_matrix(sample, 0);
    // generate a random sample: fills passed matrix, returns Y
    int Y = generate_sample(sample);
    // display sample on input screen
    display(sample, 0, 0, 1, true);

    // predict sample
    float y = nn_predict(sample, false);
    
    // train with current sample
    if(epoch < EPOCHS) {
      float loss = nn_train(sample, Y, false);
      epoch++;
      // Logging
      if(epoch % 50 == 0) {
        Serial.print("train: epoch ");
        Serial.print(epoch);
        Serial.print("/");
        Serial.print(EPOCHS);
        Serial.print(" loss: ");
        Serial.print(loss);
        Serial.print("   data: ");
        Serial.print(Y);
        Serial.print(" predict: ");
        Serial.println(y);
      }
    }
  }
    

  // update physical weights
  // delta ~ sum of distances to target position
  int delta = weights_update();
}
