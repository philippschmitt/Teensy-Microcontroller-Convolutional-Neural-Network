/*
 *  PHYSICAL NEURAL NETWORK
 *  Philipp Schmitt
 *  
*/

#include <Chrono.h>             // Time management

#define EPOCHS          1000    // def: 20000
#define L_RATE          0.2     // def: 0.001
#define LOSS_TARGET     0.05    // target loss -> network will train until reached

#define INPUT_SIZE      8
#define CONV_DEPTH      4
#define KERNEL_SIZE     3
#define CONV_SIZE       6       // divide: INPUT_SIZE / KERNEL_SIZE
#define POOLING_KERNEL  3
#define POOL_SIZE       2       // divide: CONV_SIZE / POOLING_KERNEL

float conv[CONV_DEPTH][CONV_SIZE][CONV_SIZE];
float relu[CONV_DEPTH][CONV_SIZE][CONV_SIZE];
float pool[CONV_DEPTH][POOL_SIZE][POOL_SIZE];
float lin[(POOL_SIZE*POOL_SIZE*CONV_DEPTH)];

#define TOLERANCE 30            // threshold value for nn to pot matching

#define SPEED 1               // speed up all delays by factor n. Default = 1

// Timers
Chrono timer_predict;
Chrono timer_predict_steps;

// count epochs
unsigned int epoch = 0;

int delta = 0;
int lastDelta = 0;
int predict_step = 0;

float X[8][8];
int Y;
float y;
float loss;

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
  
  weights_propagate();

  // move weights to match nn state
  Serial.println("set weights to initial neural net state");
  weights_init();

  // display_cycle();

  Serial.println("setup complete");
  Serial.println("\n");

  blink(); // loop is beginning

  timer_predict.restart();
}


void loop() {

  // PREDICT AT INTERVAL
  if(timer_predict.isRunning()) {

    // STEP 0: predict, Display X input
    if(timer_predict.hasPassed(250/SPEED) && predict_step == 0) {
      predict_step++;
      // create array for sample and fill with zeros
      set_matrix(X, 0);
      // generate a random sample: fills passed matrix, returns Y
      Y = generate_sample(X);
      // predict + calcuate loss
      y = nn_predict(X, false);
      loss = nn_loss(y,Y);
      // display sample on input screen
      // sample comes in with max value 1, but display maps to 1.6
      // -> dimming input to match conv. brightness
      display(X, 0, 0, 1.6, true);
    }

    // STEP 1: Display conv
    if(timer_predict.hasPassed(1200/SPEED) && predict_step == 1) {
      predict_step++;
      display(conv[0], 1, -1, 1, false);
      display(conv[1], 2, -1, 1, false);
      display(conv[2], 3, -1, 1, false);
      display(conv[3], 4, -1, 1, true);
    }

    // STEP 2: Display pool
    if(timer_predict.hasPassed(1300/SPEED) && predict_step == 2) {
      predict_step++;
      display(pool[3], 5, 0, 1, false);
      display(pool[2], 6, 0, 1, false);
      display(pool[1], 7, 0, 1, false);
      display(pool[0], 8, 0, 1, true);
    }

    // STEP 3: Display Lin
    if(timer_predict.hasPassed(1400/SPEED) && predict_step == 3) {
      predict_step++;
      display(lin, 9, 0, 1, true);
    }

    // STEP 4: Display Output
    if(timer_predict.hasPassed(1600/SPEED) && predict_step == 4) {
      predict_step++;
      // show if prediction was false or correct
      if((Y == 1 && y > 0) || (Y == 0 && y < 0)) {
        prediction_indicator(1);
      } else {
        prediction_indicator(2);
      }
      weights_propagate();
    }

    // STEP 5: Train
    if(timer_predict.hasPassed(4000/SPEED) && predict_step == 5) {
      predict_step++;

      // train with current sample
      if(epoch < EPOCHS || loss > LOSS_TARGET) {
        nn_train(X, y, Y, false);

        epoch++;
        // Logging
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

    // STEP 6: update weights
    if(timer_predict.hasPassed(5000/SPEED) && predict_step == 6) {
      // update physical weights after nn update
      // delta ~ sum of distances to target position
      delta = weights_update();
      Serial.println(delta);
      if(delta < TOLERANCE) {
        // move on to next step
        predict_step++;
      }
    }

    if(timer_predict.hasPassed(5000/SPEED) && predict_step == 7) {
      // stop all weights
      weights_stop();
      // fade out pixels
      display_fadeout(SPEED);
      prediction_indicator(0);
      weights_propagate();

      // restart interval
      predict_step = 0;
      timer_predict.restart();
    }
  }



  /*

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

    // delta = weights_update();

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
  */

}
