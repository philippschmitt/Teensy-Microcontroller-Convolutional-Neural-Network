# Physical Neural Net
A working implementation of a convolutional neural network written in Arduino C for Teensy 3.6.

Thanks to [https://github.com/Atcold](Alfredo Canziani) for explaining the math to me!

## Architecture:
```
0   input     8x8 px
1   conv.     4x kernel size=3, stride=1, pad=0
2   maxpool   4x 2x2 px ReLu
3   linear
4   output
```

## Usage
The neural net implementation itself is in _neuralnet.ino_: `nn_predict()` computes the gradient for a single sample. `nn_train()` completes a single training step.

It includes the necessary functions like `nn_conv()` for a forward pass, `nn_conv_backwards()` to calculate gradient descent loss, and `nn_update_conv()` to update weights.

See `diagram.ino` for general operating and order of commands. The file includes other code: it's for [an interactive art piece]([url](https://philippschmitt.com/work/mark-ii-convolutional-neural-network)) that includes LED displays for layer outputs as well as motorized potentiometers that encode the network's weights.

Note: Initial weights are pre-generated in `Network training.ipynb` (for suitable distribution) and hardcoded on the Teensy.

