# Physical Neural Net
A working implementation of a convolutional neural network written in Arduino C for Teensy 3.6

## Architecture:
```
0   input     8x8 px
1   conv.     4x kernel size=3, stride=1, pad=0
2   relu
2   maxpool   4x 2x2 px ReLu
3   linear
4   output
```

## Usage
Initial weights are pre-generated in `Network training.ipynb` (for suitable distribution) and hardcoded on the Teensy.

