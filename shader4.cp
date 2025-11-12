#version 430 core

layout(local_size_x = 10, local_size_y = 1, local_size_z = 1) in;


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
uniform float alfa;
uniform float dt;
uniform float totalTime;
uniform int nodeCount;
uniform uint randomSeed;
uniform int startNodeIndex;
uniform float D;

shared float S[10];  
shared float I[10];
shared float newI[10];
shared float newS[10]; 

uint rngState;

void initRNG(uint seed){
	rngState = seed;
}

float random() {
    rngState = rngState * 1664525u + 1013904223u;
    return float(rngState) / 4294967296.0;
}

int randomInt(int minVal, int maxVal) {
    if (maxVal <= minVal) return minVal;
    return minVal + int(random() * float(maxVal - minVal));
}

void meet(uint idx){
    if(idx >= nodeCount) return;

    int s = S[idx];
    int i = I[idx];
    int total = s + i;

    int person1 = randomInt(0, total);
    int person2 = randomInt(0, total);

    if(person1 < s && (person2 >= s && person2 < total)){
        float infProb = random();

        if(infProb < infectionRate){
            S[idx]--;
            I[idx]++;
        }
    }
}

void recover(uint idx){
    if(idx >= nodeCount) return;
            
    int i = I[idx];
    int person = randomInt(0, i);

    float recProb = random();
    if(recProb < alfa){
            S[idx]++;
            I[idx]--;
    }
}

void diffuse(uint idx){

	if(idx >= nodeCount) return;

    newI[idx] = 0;
    newS[idx] = 0;


	int degree = degrees[idx];
    if(degree == 0) return;

    int s = S[idx];
    int i = I[idx];
    int total = s + i;
    if(total == 0) return;

    int targetNeighborIdx = randomInt(0, degree);
    int count = 0;
    int neighborNode = -1;
    
    for(int j = 0; j < nodeCount; j++){
        if(adjacency[idx * nodeCount + j] == 1){
            if(count == targetNeighborIdx){
                neighborNode = j;
                break;
            }
            count++;
        }
    }
    
    if(neighborNode == -1) return;
    
    float diff = random();
    if(diff < D){
        int person = randomInt(0, total);
        if(person < s){
            // Move S
            atomicAdd(newS[idx], -1);
            atomicAdd(newS[neighborNode], 1);
        } else {
            // Move I
            atomicAdd(newI[idx], -1);
            atomicAdd(newI[neighborNode], 1); 
        }
    }
    
    //Apply deltas
    S[idx] += newS[idx];
    I[idx] += newI[idx]; 
} 

void main(){
	
	uint idx = gl_GlobalInvocationID.x;

	if (idx >= nodeCount) return;


    initRNG(randomSeed + startNodeIndex * 7919u);
    
    S[idx] = results[idx * 2 + 0];
    I[idx] = results[idx * 2 + 1];
	barrier();

    
    // float t = 0.0;
    // while (t < totalTime) {
    //     meet(idx);

    //     recover(idx);
    //     barrier();
    //     diffuse(idx);

    //     t += dt;
    // }

    lastResults[idx * 2 + 0] = S[idx]; 
    lastResults[idx * 2 + 1] = I[idx]; 
    // lastResults[neighborNode * 2 + 0] = S[neighborNode];
    // lastResults[neighborNode * 2 + 1] = I[neighborNode];
}
