/*
 * 	MAIN NEURAL NETWORK IMPLEMENTATION
 *	
 *	Architecture:
 *	0 	input			8x8 px
 *	1 	conv.			4x kernel size=3, stride=1, pad=0 		+ 4x bias
 *  2		relu
 * 	2 	maxpool		4x 2x2 px ReLu (?)											4x bias
 *	3 	linear																					1x bias
 *
*/

#include <math.h>
#define EULER 2.718281828459045235360287471352

// for now, weights are initialized with distribution copied from pytorch currently

float conv_weights[CONV_DEPTH][KERNEL_SIZE][KERNEL_SIZE] = {
	{
	  { 0.1829907 , -0.04207245,  0.01272717},
	  { 0.07723489,  0.2067922 ,  0.3200647 },
	  {-0.25687453, -0.12215659,  0.13100329}
	},
	{
	  { 0.27618316,  0.29006913,  0.2941189 },
	  { 0.06633818, -0.2898609 ,  0.03066418},
	  {-0.20853512, -0.3106514 ,  0.29616418}
	},
	{
	  { 0.25345328, -0.33250934,  0.06239069},
	  {-0.05615333, -0.05485371, -0.15258563},
	  { 0.12818539, -0.19743451,  0.12219712}
	},
	{
	  { 0.16856936,  0.23862389,  0.12463704},
	  {-0.32991177, -0.2162323 ,  0.16643834},
	  { 0.06976712, -0.260028  , -0.19193983}
	}
};
float conv_bias[CONV_DEPTH] = { 0.3135831,  0.22460595, -0.14534172, -0.08389494 };
float conv[CONV_DEPTH][6][6];
float relu[CONV_DEPTH][6][6];
float pool[CONV_DEPTH][2][2];

float flatten_dy[CONV_DEPTH][2][2];
float pool_dy[CONV_DEPTH][6][6];
float relu_dy[CONV_DEPTH][6][6];

float lin_weights[16] = {-0.23814952, -0.00449353, -0.18826473, -0.19283918, -0.0137749, 0.03753626, -0.10238257,  0.1483444 , -0.15213478,  0.22684252, 0.17132497, -0.21082073, -0.06222108,  0.01128066,  0.03647527, 0.05929357};
float lin_bias[1] = {0.09810707};
float lin[16];
float lin_dy[16]; // derivative of lin with respect to y

// vars for weight gradients
float lin_dw[16]; // derivative of lin with respect to w(eights)
float conv_dw[CONV_DEPTH][KERNEL_SIZE][KERNEL_SIZE];

// setup neural net
void nn_setup() {

}


void nn_conv(float (*X)[INPUT_SIZE], float (*out)[6][6]) {
	// go over depth / 4 activations
	for(int d=0; d<CONV_DEPTH; d++){
		// slide kernel over X
		for(int y=0; y<CONV_SIZE; y++){
			for(int x=0; x<CONV_SIZE; x++){
				float sum = 0.;
				// compute weighted sum for all input kernel weight pairs
				for(int cy=0; cy<KERNEL_SIZE; cy++){
					for(int cx=0; cx<KERNEL_SIZE; cx++){
						sum += X[y+cy][x+cx] * conv_weights[d][cy][cx];
					}
				}
				// save activation
				out[d][y][x] = sum + conv_bias[d];
			}   
		}
	}
}

void nn_conv_backwards(float (*dy)[6][6], float (*X)[INPUT_SIZE], float (*dw)[KERNEL_SIZE][KERNEL_SIZE]) {
	// zero the dw tensor
	set_matrix(dw, 0);
	// go over depth / 4 activations
	for(int d=0; d<CONV_DEPTH; d++) {
		// go over kernel
		for(int fy=0; fy<KERNEL_SIZE; fy++){
			for(int fx=0; fx<KERNEL_SIZE; fx++){
				// compute dw, sum over input * dy
				for(int y=0; y<6; y++){
					for(int x=0; x<6; x++){
						dw[d][fy][fx] += dy[d][y][x] * X[y+fy][x+fx];
					}
				}
			}
		}
	}
}


void nn_relu(float (*in)[6][6], float (*out)[6][6]) {
	// go over activations
	for(int d=0; d<CONV_DEPTH; d++){
    for(int y=0; y<CONV_SIZE; y++){
    	for(int x=0; x<CONV_SIZE; x++){
    		out[d][y][x] = max(0, in[d][y][x]);
  	 	}
    }
	}
}

void nn_relu_backwards(float (*dx)[6][6], float (*X)[6][6], float (*dy)[6][6]) {
	// go over activations
	for(int d=0; d<CONV_DEPTH; d++){
    for(int y=0; y<CONV_SIZE; y++){
    	for(int x=0; x<CONV_SIZE; x++){
    		dy[d][y][x] = dx[d][y][x]; // copy dx to dy
    		if(X[d][y][x] <= 0) {
    			dy[d][y][x] = 0;
    		}
  	 	}
    }
	}
}


void nn_pool(float (*in)[6][6], float (*out)[2][2]) {
	// go over activations
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<(CONV_SIZE/POOLING_KERNEL); y++){
			for(int x=0; x<(CONV_SIZE/POOLING_KERNEL); x++){
				// at this point, 6x6 activation is split in four 2x2 subsections
				float p = 0.;
				// now maxpool for each subsection
				for(int py=0; py<POOLING_KERNEL; py++){
					for(int px=0; px<POOLING_KERNEL; px++){
						p = max(p, in[d][y*POOLING_KERNEL+py][x*POOLING_KERNEL+px]);
					}
				}
				// save activation
				out[d][y][x] = p;
			}
		}
	}
}

// g 	incoming gradient
// x  input of pool layer
// y  output of pool layer
// dy gradient to pass on to prev layer (i.e. relu)
void nn_pool_backwards(float (*g)[2][2], float (*X)[6][6], float (*Y)[2][2], float (*dy)[6][6]) {
	// match pool output y with pool input x to find max values
	// could cache pool positions during inference, but what does it matter ...
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<CONV_SIZE/POOLING_KERNEL; y++){
			for(int x=0; x<CONV_SIZE/POOLING_KERNEL; x++){
				// helper: if there are multiple identical max values, we set done = true after the first one,
				// to skip to the next pool subsection
				bool done = false;
				// iterate over pool input with kernel
				for(int py=0; py<POOLING_KERNEL; py++){
					if(done) {
						break; 
					}
					for(int px=0; px<POOLING_KERNEL; px++){
						if(done) { break; }
						if(X[d][y*POOLING_KERNEL+py][x*POOLING_KERNEL+px] == Y[d][y][x]) {
							dy[d][y*POOLING_KERNEL+py][x*POOLING_KERNEL+px] = g[d][y][x];
							done = true;
						} else {
							dy[d][y*POOLING_KERNEL+py][x*POOLING_KERNEL+px] = 0;
						}
					}
				}
			}
		}
	}
}


void nn_flatten(float (*in)[2][2], float *out) {
	int i = 0;
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<CONV_SIZE/POOLING_KERNEL; y++){
			for(int x=0; x<CONV_SIZE/POOLING_KERNEL; x++){
				// add to 0 so it does a real copy of val
				out[i] = in[d][y][x];
				i++;
			}
		}
	}
}

void nn_flatten_backwards(float *X, float (*dy)[2][2]) {
	int i = 0;
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<CONV_SIZE/POOLING_KERNEL; y++){
			for(int x=0; x<CONV_SIZE/POOLING_KERNEL; x++){
				dy[d][y][x] = X[i];
				i++;
			}
		}
	}
}


float nn_linear(float *in) {
	float out = 0;
	// y=xAT+b
	for(int i=0; i<16; i++){
		out += in[i] * lin_weights[i];
	}
	out += lin_bias[0];
	return out;
}

void nn_linear_backwards(float dL, float *W, float *X, float *dy, float *dW) {
	// compute dy, dW
	for(int i=0; i<16; i++){
		dy[i] = W[i] * dL;
		dW[i] = X[i] * dL;
	}
}


float sigmoid(float x) {
	return 1 / (1 + pow(EULER, -x));
}

// BCE loss with logits
float nn_loss(float y, float Y) {
	float max_val = max(0, -y);
	//(1-Y) * y + max_val + math.log((math.e ** -max_val) + math.e ** (-y-max_val))
	return (1-Y) * y + max_val + logf( pow(EULER, -max_val) + pow(EULER, -y-max_val) );
}

float nn_loss_backwards(float y, float Y) {
	return (sigmoid(y) - Y) * 1;
}



// compute gradient for a single sample
float nn_predict(float (*X)[INPUT_SIZE], bool log) {

	nn_conv(X, conv);
	//display(conv[1], -1, 1);
	
	nn_relu(conv, relu);
	// display(relu[2], 0, 1);

	// Logging
	if(log) {
		Serial.println(F("relu"));
		for(int y=0; y<6; y++){
			for(int x=0; x<6; x++){
				Serial.print(relu[1][y][x], 4);
				Serial.print(" ");
			}
			Serial.println("");
		}
	}

	nn_pool(relu, pool);
	display(pool, 0, 3);
	delay(200);

	// Logging
	if(log) {
		Serial.println(" ");
		Serial.println(F("pool"));
		for(int y=0; y<2; y++){
			for(int x=0; x<2; x++){
				Serial.print(pool[1][y][x], 4);
				Serial.print(" ");
			}
			Serial.println("");
		}
	}

	nn_flatten(pool, lin);
	
	// Logging
	if(log) {
		Serial.println(" ");
		Serial.println(F("flatten"));
		for(int i=0; i<16; i++){
			Serial.println(lin[i], 4);
		}
	}

	float y = nn_linear(lin);

	// Logging
	if(log) {
		Serial.println(" ");
		Serial.println(y, 4);
		if(y > 0) {
			Serial.println("Predict C");
		} else {
			Serial.println("Predict D");
		}
	}

	return y;
}



float nn_train(float (*X)[INPUT_SIZE], float Y, bool logging) {
	// predict y
	float y = nn_predict(X, false);
	// Loss
	float L = nn_loss(y,Y);
	// Derivative of Loss
	float dL = nn_loss_backwards(y,Y);

	// compute the gradient
	nn_linear_backwards(dL, lin_weights, lin, lin_dy, lin_dw);
	nn_flatten_backwards(lin_dy, flatten_dy);
	nn_pool_backwards(flatten_dy, relu, pool, pool_dy);
	nn_relu_backwards(pool_dy, conv, relu_dy);
	nn_conv_backwards(relu_dy, X, conv_dw);

	// update weights
	nn_update_lin(lin_weights, lin_dw, dL);
	nn_update_conv(conv_weights, conv_dw, conv_bias, relu_dy);

	// Log progress to console
  Serial.print("pred: ");
  Serial.print(y);
  Serial.print(" Y: ");
  Serial.print(Y);
  Serial.print(" ");

  // display(conv_weights, -1, 1);

	return L;
}


void nn_update_lin(float *W, float *dw, float dL) {
	for(int i=0; i<((CONV_SIZE/POOLING_KERNEL)^2*CONV_DEPTH); i++){
		W[i] -= L_RATE * dw[i];
	}

	// bias: db = dL
	lin_bias[0] -= dL * L_RATE;
}


void nn_update_conv(float (*W)[KERNEL_SIZE][KERNEL_SIZE], float (*dw)[KERNEL_SIZE][KERNEL_SIZE], float (*b), float (*dy)[6][6]) {
	// update weights
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<KERNEL_SIZE; y++){
		  for(int x=0; x<KERNEL_SIZE; x++){
		  	W[d][y][x] -= dw[d][y][x] * L_RATE;
		  }
		}
	}
	// update bias
	for(int d=0; d<CONV_DEPTH; d++){
		float db = 0;
		for(int y=0; y<6; y++){
		  for(int x=0; x<6; x++){
		  	db += dy[d][y][x];
		  }
		}
		b[d] -= db * L_RATE;
	}
}



