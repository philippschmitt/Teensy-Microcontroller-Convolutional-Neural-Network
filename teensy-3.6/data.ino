/*
 *	WORKING WITH DATA
 *	- generate input samples
*/

#define N_SAMPLES		4


const float sample_C_a[6][4] = {
	{0,1,1,1},
	{1,0,0,0},
	{1,0,0,0},
	{1,0,0,0},
	{1,0,0,0},
	{0,1,1,1}
};

const float sample_C_b[4][3] = {
	{1,1,1},
	{1,0,0},
	{1,0,0},
	{1,1,1}
};

const float sample_D_a[6][4] = {
	{1,1,1,0},
	{1,0,0,1},
	{1,0,0,1},
	{1,0,0,1},
	{1,0,0,1},
	{1,1,1,0}
};

const float sample_D_b[4][3] = {
	{1,1,0},
	{1,0,1},
	{1,0,1},
	{1,1,0},
};

// stores size information for all character samples, in order
// y,x, Y(char)
const int sample_dims[][3] = {
	{6,4, 1},
	{4,3, 1},
	{6,4, 0},
	{4,3, 0}
};


// Work in progress
// target:
//  0 = D
//	1 = C
int generate_sample(float (*matrix)[8]) {
	// select rand sample and place randomly on canvas
	int sample = floor(random(0,N_SAMPLES));
	// def. rand x position
	//int x_minMax[] = {0, 8-sample_dims[sample][1]};
	int x_pos = round(random(0, 8-sample_dims[sample][1]));
	int y_pos = round(random(0, 8-sample_dims[sample][0]));
	// place letter in 8x8 matrix
	for(int y=0; y < sample_dims[sample][0]; y++) {
		for(int x=0; x<sample_dims[sample][1]; x++) {
			// pick sample data. annoying that this is in the loop, 
			// but copying multi-dim arrs is annoying in C
			switch(sample) {
				case 0: matrix[y+y_pos][x+x_pos] = sample_C_a[y][x]; break;
				case 1: matrix[y+y_pos][x+x_pos] = sample_C_b[y][x]; break;
				case 2: matrix[y+y_pos][x+x_pos] = sample_D_a[y][x]; break;
				case 3: matrix[y+y_pos][x+x_pos] = sample_D_b[y][x]; break;
			}

		}
	}
	// return Y
	return sample_dims[sample][2];
}


// Fill tensor with set value
// Copies for different tensor dimensions
// 1x8x8
void set_matrix (float (*matrix)[8], float value) {
	for(int y=0; y<8; y++){
		for(int x=0; x<8; x++){
			matrix[y][x] = value;     
		}   
	}
}
// 4x6x6
void set_matrix (float (*matrix)[CONV_SIZE][CONV_SIZE], float value) {
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<6; y++){
			for(int x=0; x<6; x++){
				matrix[d][y][x] = value;     
			}   
		}
	}
}
// 4x3x3
void set_matrix (float (*matrix)[KERNEL_SIZE][KERNEL_SIZE], float value) {
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<KERNEL_SIZE; y++){
			for(int x=0; x<KERNEL_SIZE; x++){
				matrix[d][y][x] = value;     
			}   
		}
	}
}
// 4x2x2
void set_matrix (float (*matrix)[CONV_SIZE/POOLING_KERNEL][CONV_SIZE/POOLING_KERNEL], float value) {
	for(int d=0; d<CONV_DEPTH; d++){
		for(int y=0; y<CONV_SIZE/POOLING_KERNEL; y++){
			for(int x=0; x<CONV_SIZE/POOLING_KERNEL; x++){
				matrix[d][y][x] = value;     
			}   
		}
	}
}
// 1x16
void set_matrix(float *matrix, float value) {
	for(int i=0; i<16; i++){
		matrix[i] = value;
	}
}


