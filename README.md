# A convolutional neural network implemented on a Teensy 3.6 Microcontroller
A working implementation of a convolutional neural network written in Arduino C for Teensy 3.6.

The code in this repository was written for [an interactive art piece](https://philippschmitt.com/work/mark-ii-convolutional-neural-network). It is not a reusable template for your own project, sorry! I'm open sourcing it nonetheless hoping it might be helpful to someone.

Thanks to [Yann LeCun](http://yann.lecun.com) for guidance and to [Alfredo Canziani](https://github.com/Atcold) for explaining the math to me!


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

Note: Initial weights are pre-generated in `Network training.ipynb` and hardcoded.

See `diagram.ino` for general operating and order of commands. The file includes other code: LED displays for layer outputs as well as motorized potentiometers that encode the network's weights.
