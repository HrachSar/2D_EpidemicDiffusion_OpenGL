#ifndef RENDERER_H
#define RENDERER_H

#include "resource_manager.h"
#include <vector>

#define N 500
#define M0 4
#define M 4
#define M_PI 3.14
#define BETA 0.05
#define D 0.4

const float FOV = 45.0f;
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;


typedef struct
{
    float x, y;
    float infected;
    float susceptible;
    int edge_list[N];
} Node;

const int NODE_STRIDE = sizeof(Node);


class renderer {
public:
    unsigned int m_VBO, m_VAO, m_EBO, m_edgeVAO, m_edgeVBO;//Buffers
    unsigned int m_lastResultsSSBO, m_adjacencySSBO, m_resultsSSBO, m_degreeSSBO;
    unsigned int m_computeID;
    glm::mat4 m_model, m_view, m_projection; // matrices
    std::string m_name;
    shader m_shader;
    const char* m_vertexShader; // path to vertex shader
    const char* m_computeShader;
    const char* m_fragmentShader; //path to fragment shader
    const char* m_texture;
    int m_nodePool[N] = { 0 };
    int m_totalDegree;
    int m_startNode;
    bool m_simulationsRun;
    std::vector<Node> m_graph;
    std::vector<float> m_resultsData;
    std::vector<float> m_lastResults;

    renderer(const char* name, int startNode, const char* vShader, const char* fShader, const char* cShader, const char* texture = nullptr, glm::mat4 model = glm::mat4(1.0f), glm::mat4 view = glm::mat4(1.0f), glm::mat4 projection = glm::mat4(1.0f));
    ~renderer();

    void init();
    void add_node(int i, int j);
    void generate();
    void render();
    void runAllSimulations(float dt);
    int getInfectedCountForStartNode(int startNode);
    void free();
    void getSimulationResults(int startNode, float* susceptibleStates, float* infectedStates);
    void setViewMatrix(glm::mat4 matrix);
};



#endif //RENDERER_H
