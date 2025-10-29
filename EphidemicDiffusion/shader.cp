#version 430 core

layout(local_size_x = 500, local_size_y = 1, local_size_z = 1) in;


layout(std430, binding = 1) readonly buffer AdjacencyBuffer{
	int adjacency[];
};
layout(std430, binding = 2) readonly buffer DegreeBuffer{
	int degrees[];
};
layout(std430, binding = 3) buffer ResultsBuffer{
	float results[];
};
layout(std430, binding = 4) buffer LastResultsBuffer{
	float lastResults[];
}; 

uniform float infectionRate;
uniform float dt;
uniform float totalTime;
uniform int nodeCount;
uniform uint randomSeed;
uniform int startNodeIndex;
uniform float D;

shared float k1x[500], k1y[500];
shared float k2x[500], k2y[500];
shared float k3x[500], k3y[500];
shared float k4x[500], k4y[500];
shared float temp_x[500], temp_y[500];
shared float S[500];  
shared float I[500];
shared float newI[500];

uint rngState;

void initRNG(uint seed){
	rngState = seed;
}

float random() {
    rngState = rngState * 1664525u + 1013904223u;
    return float(rngState) / 4294967296.0;
}

void df(in float S[500], in float I[500], out float dS[500], out float dI[500], uint idx){

	if(idx >= nodeCount) return;

	float infectedNeighbours = 0.0;
	int degree = 0;

	for(int j = 0; j < nodeCount; j++){
		if(adjacency[idx * nodeCount + j] == 1){
			infectedNeighbours += I[j];
		}
	}

	degree = degrees[idx];
	if (degree > 0){
		dI[idx] = infectionRate * S[idx] * infectedNeighbours / float(degree);
		dS[idx] = -infectionRate * S[idx] * infectedNeighbours / float(degree);
	}else{
		dI[idx] = 0;
		dS[idx] = 0;
	}
}

void rk4_step(inout float S[500], inout float I[500], uint idx, float h){

	df(S, I, k1x, k1y, idx);
	barrier();

	temp_x[idx] = S[idx] + 0.5 * k1x[idx] * h;
	temp_y[idx] = I[idx] + 0.5 * k1y[idx] * h;
	barrier();

	df(temp_x, temp_y, k2x, k2y, idx);
	barrier();

	temp_x[idx] = S[idx] + 0.5 * k2x[idx] * h;
	temp_y[idx] = I[idx] + 0.5 * k2y[idx] * h;
	barrier();

	df(temp_x, temp_y, k3x, k3y, idx);
	barrier();

	temp_x[idx] = S[idx] + k3x[idx] * h;
	temp_y[idx] = I[idx] + k3y[idx] * h;
	barrier();

	df(temp_x, temp_y, k4x, k4y, idx);
	barrier();

	float x = (h / 6.0) * (k1x[idx] + 2 * k2x[idx] + 2 * k3x[idx] + k4x[idx]);
	float y = (h / 6.0) * (k1y[idx] + 2 * k2y[idx] + 2 * k3y[idx] + k4y[idx]);

	S[idx] += x;
	I[idx] += y;

	S[idx] = clamp(S[idx], 0.0, 1.0);
    I[idx] = clamp(I[idx], 0.0, 1.0);
	barrier();
}

void diffuse(inout float S[500], inout float I[500], uint idx, float h){

	if(idx >= nodeCount) return;

	float diff = 0.0;
	int degree = 0;
	for(int j = 0; j < nodeCount; j++){
		if(adjacency[idx * nodeCount + j] == 1){
			degree++;
			diff += I[idx] - I[j];
		}
	}
	if(degree > 0){
		diff /= float(degree);
	}
	newI[idx] = I[idx] + diff * D * h;
	barrier();

	I[idx] = clamp(newI[idx], 0.0, 1.0);
	S[idx] = clamp(1 - I[idx], 0.0, 1.0);

	barrier();
}

void main(){
	
	uint idx = gl_GlobalInvocationID.x;

	if(idx < 0 || idx >= nodeCount) return;


    initRNG(randomSeed + startNodeIndex * 7919u);
    
    S[idx] = results[idx * 2 + 0];
    I[idx] = results[idx * 2 + 1];
	barrier();

    
    float t = 0.0;
    while (t < totalTime) {
        rk4_step(S, I, idx, dt);
		diffuse(S, I, idx, dt);
        t += dt;
    }

    lastResults[idx * 2 + 0] = S[idx]; 
    lastResults[idx * 2 + 1] = I[idx]; 

}

