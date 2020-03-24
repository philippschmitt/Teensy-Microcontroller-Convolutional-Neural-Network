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
	  { 0.2569,  0.1105,  0.1674},
	  { 0.1287, -0.1043,  0.5103},
	  {-0.1522, -0.2983,  0.5761}
	},
	{
	  { 0.4487,  0.4797,  0.4105},
	  {-0.2345, -0.3460, -0.2620},
	  {-0.3920, -0.3163,  0.0459}
	},
	{
	  { 0.4300, -0.3331,  0.0822},
	  {-0.1247, -0.1019, -0.1554},
	  { 0.0019, -0.2639,  0.0248}
	},
	{
	  { 0.4221,  0.3933,  0.2805},
	  {-0.4268, -0.3325,  0.0364},
	  {-0.0887, -0.3105, -0.3442}
	}
};
float conv_bias[CONV_DEPTH] = {0.2463,  0.2489, -0.0247,  0.0242};
float conv[CONV_DEPTH][6][6];
float relu[CONV_DEPTH][6][6];
float pool[CONV_DEPTH][2][2];

float lin_weights[16] = { -0.5481, -0.3763, -0.2324, -0.3884,  0.1324,  0.7295,  0.0358,  0.1847, -0.1555,  0.3133,  0.0970, -0.3748,  0.1187,  0.6135,  0.2777,  0.2629 };
float lin_bias[1] = {0.1862};
float lin[16];


// setup neural net
void nn_setup() {

}


// compute gradient for a single sample
void nn_predict(float (*X)[INPUT_SIZE], bool log) {

	// output
	float y = 0;

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
	if(log) {
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

	y = nn_linear(lin);

	// Logging
	if(log) {
		Serial.println(" ");
		Serial.println(y, 4);
		if(y > 0) {
			Serial.println("Prediction: " + "C");
		} else {
			Serial.println("Prediction: " + "D");
		}
	}

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

