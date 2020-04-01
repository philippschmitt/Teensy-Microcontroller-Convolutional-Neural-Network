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
	// set Y
	return sample_dims[sample][2];
}


// right now this is hard coded for 8x8 input matrix.
// will extend to allow for other dims if necessary
void set_matrix (float (*matrix)[8], float value) {
	for(int y=0; y<8; y++){
		for(int x=0; x<8; x++){
			matrix[y][x] = value;     
		}   
	}
}


