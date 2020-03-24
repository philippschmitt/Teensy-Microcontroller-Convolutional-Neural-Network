# Physical Neural Net

*work-in-progress*

## Architecture
- Teensy 3.6
- Adafruit NeoPixels (currently only one 8x8 matrix, connected to `PIN 0`)

## Usage
Weights are pre-trained in `Network training.ipynb` and hardcoded on the Teensy. Current model is not suuuper accurate. Need to work on this some more.

#### 1. Generate sample letter
```
// create array for sample and fill with zeros
float sample[8][8];
set_matrix(sample, 0);
// generate a random sample
generate_sample(sample);
```

#### 2. Display sample letter using `display_8x8(sample)`

#### 3. Predict using `nn_predict(sample, log)`

- sample is 8x8 matrix
- log=true enables logging all activations / intermediate layer outputs to console for debugging