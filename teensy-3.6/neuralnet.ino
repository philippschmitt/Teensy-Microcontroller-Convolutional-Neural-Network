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

#define EPOCHS 					10000
#define L_RATE 					0.001
#define INPUT_SIZE			8
#define CONV_DEPTH			4
#define KERNEL_SIZE 		3
#define CONV_SIZE				6 			// divide: INPUT_SIZE / KERNEL_SIZE
#define POOLING_KERNEL 	3


// for now, weights are initialized with distribution copied from pytorch currently

float conv_weights[CONV_DEPTH][KERNEL_SIZE][KERNEL_SIZE] = {
	{
	  { 0.2583,  0.1130,  0.1664},
	  { 0.1290, -0.1066,  0.5152},
	  {-0.1506, -0.3007,  0.5802}
	},
	{
	  { 0.4498,  0.4808,  0.4108},
	  {-0.2379, -0.3478, -0.2646},
	  {-0.3933, -0.3176,  0.0442}
	},
	{
	  { 0.4306, -0.3333,  0.0822},
	  {-0.1255, -0.1022, -0.1554},
	  { 0.0010, -0.2646,  0.0243}
	},
	{
	  { 0.4248,  0.3947,  0.2808},
	  {-0.4287, -0.3338,  0.0354},
	  {-0.0901, -0.3116, -0.3454}
	}
};
float conv_bias[CONV_DEPTH] = {0.2444,  0.2503, -0.0244,  0.0267};
float conv[CONV_DEPTH][6][6];
float relu[CONV_DEPTH][6][6];
float pool[CONV_DEPTH][2][2];

float lin_weights[16] = { -0.5527, -0.3821, -0.2334, -0.3901,  0.1349,  0.7348,  0.0367,  0.1860, -0.1553,  0.3138,  0.0966, -0.3762,  0.1210,  0.6184,  0.2790,  0.2644 };
float lin_bias[1] = {0.1875};
float lin[16];


// setup neural net
void nn_setup() {

}


// compute gradient for a single sample
float nn_predict(float (*X)[INPUT_SIZE], bool log) {

	nn_convolve(X, conv);
	display(conv[2], -1, 1);
	
	nn_relu(conv, relu);
	display(relu[2], 0, 1);

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


void nn_convolve(float (*X)[INPUT_SIZE], float (*out)[6][6]) {
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


float nn_linear(float *in) {
	float out = 0.;
	// y=xAT+b
	for(int i=0; i<16; i++){
		out += in[i] * lin_weights[i];
	}
	out += lin_bias[0];
	return out;
}


// pseudocode for now
void nn_train(float (*X)[INPUT_SIZE], float Y) {
	// predict y
	float y = nn_predict(X, false);
	float loss = nn_loss(y,Y);
	
	Serial.println(y);
	Serial.println(Y);
	Serial.println(loss, 6);
	Serial.println(" ");

	/*
	// calculate error
	nn_loss();
	// backpropagate
	nn_backprop();
	// update weights
	nn_update();

	*/
}


float sigmoid(float y) {
	return 1 / (1 + pow(EULER, -y));
}

// # def binary_cross_entropy(input, y): return - (pred.log()*y + (1-y)*(1-pred).log()).mean()
// no mean for now, because it's a single input atm
// will expand later
float binary_cross_entropy(float y, float Y) {
	return - (log(y)*Y + (1-Y)*log(1-y));
}

// BCE loss with logits
float nn_loss(float y, float Y) {
	return binary_cross_entropy(sigmoid(y), Y);
}





