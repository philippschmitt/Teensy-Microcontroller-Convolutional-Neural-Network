/*
 *  PHYSICAL NEURAL NETWORK
 *  Philipp Schmitt
 *  
*/

#include <Metro.h> // Include the Metro library for timers

#define EPOCHS          1000       // def: 20000
#define L_RATE          0.2        // def: 0.001
#define LOSS_TARGET     0.05        // target loss -> network will train until reached

#define INPUT_SIZE      8
#define CONV_DEPTH      4
#define KERNEL_SIZE     3
#define CONV_SIZE       6       // divide: INPUT_SIZE / KERNEL_SIZE
#define POOLING_KERNEL  3
#define POOL_SIZE       2       // divide: CONV_SIZE / POOLING_KERNEL

#define TOLERANCE 30               // threshold value for nn to pot matching

// Timers
Metro neuralnetTimer = Metro(3500);
Metro potReadTimer = Metro(10);

// count epochs
unsigned int epoch = 0;

int delta = 0;
int lastDelta = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("hello!");
  // blink to signal
  blink();
  // setup neopixels
  display_setup();
  
  // _test() is a short flash, _cycle() goes through all LEDs
  Serial.println("Test Display");
  display_test();

  // initialize the neural net
  nn_setup();

  // setup phyiscal weights
  // set calibrate to false for faster boot
  weights_setup(false);

  // move weights to match nn state
  Serial.println("set weights to initial neural net state");
  weights_init();

  // display_cycle();

  Serial.println("setup complete");
  Serial.println("\n");

  blink(); // loop is beginning
}


void loop() {


  if( neuralnetTimer.check() == 1 ) {

    // create array for sample and fill with zeros
    float sample[8][8];
    set_matrix(sample, 0);
    // generate a random sample: fills passed matrix, returns Y
    int Y = generate_sample(sample);

    // stop all weights
    weights_stop();

    // predict sample
    float y = nn_predict_slow(sample, false);
    float loss = nn_loss(y,Y);

    // show if prediction was false or correct
    if((Y == 1 && y > 0) || (Y == 0 && y < 0)) {
      prediction_indicator(1);
    } else {
      prediction_indicator(2);
    }
    weights_propagate();

    delta = weights_update();
    
    // fade out pixels
    display_fadeout();
    prediction_indicator(0);
    weights_propagate();

    
    // train with current sample
    if(epoch < EPOCHS || loss > LOSS_TARGET) {
      nn_train(sample, y, Y, false);
      epoch++;
      // Logging
      //if(epoch % 20 == 0) {
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
      //}

    // after training is done, visitors can interact and tune weights
    } else {
      // propagate physical status to neural network
      // weights_import();
    }
  }
  
  // update physical weights
  // delta ~ sum of distances to target position
  delta = weights_update();
}
