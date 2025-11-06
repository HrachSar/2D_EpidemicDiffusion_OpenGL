#include "renderer.h"

float clamp(float x, float min_val, float max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

renderer::renderer(const char* name, int startNode, const char* vShader, const char* fShader, const char* cShader, const char* texture, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    m_name = name;
    m_vertexShader = vShader;
    m_computeShader = cShader;
    m_fragmentShader = fShader;
    m_computeShader = cShader;
    m_texture = texture;
    m_model = model;
    m_view = view;
    m_projection = projection;
    m_totalDegree = 0;
    m_startNode = startNode;
    m_simulationsRun = false;
}

renderer::~renderer() {
    free();
}

void renderer::init() {
    m_graph.resize(N);

    // Initialize node pool to zero
    for (int i = 0; i < N; i++) {
        m_nodePool[i] = 0;
        for (int j = 0; j < N; j++) {
            m_graph[i].edge_list[j] = 0;
        }
    }

    // Create initial clique (M0 fully connected nodes)
    for (int i = 0; i < M0; i++) {
        for (int j = i + 1; j < M0; j++) {
            m_graph[i].edge_list[j] = 1;
            m_graph[j].edge_list[i] = 1;
            m_nodePool[i]++;
            m_nodePool[j]++;
            m_totalDegree += 2;
        }
    }

    // Set node positions in circular layout (better for graph visualization)
    float radius = 0.8f;
    for (int i = 0; i < N; i++) {
        float angle = 2.0f * M_PI * i / N;
        m_graph[i].x = radius * cosf(angle);
        m_graph[i].y = radius * sinf(angle);
    }

    resource_manager::loadShader(m_vertexShader, m_fragmentShader, nullptr, m_computeShader, m_name);
    if (m_texture != nullptr)
        resource_manager::loadTexture(m_texture, true, m_name);

    m_shader = resource_manager::getShader(m_name);
    m_shader.use();
    m_computeID = m_shader.computeID;
    m_projection = glm::perspective(glm::radians(FOV), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    m_shader.setMat4x4("projection", m_projection);

    // Setup vertex array for nodes
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_graph.size() * sizeof(Node), m_graph.data(), GL_DYNAMIC_DRAW);

    // Position attribute (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Node), (void*)offsetof(Node, x));
    glEnableVertexAttribArray(0);

    //infected 
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Node), (void*)offsetof(Node, infected));
    glEnableVertexAttribArray(1);
    //susceptible
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Node), (void*)offsetof(Node, susceptible));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup VAO/VBO for edges (lines)
    glGenVertexArrays(1, &m_edgeVAO);
    glBindVertexArray(m_edgeVAO);

    glGenBuffers(1, &m_edgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_edgeVBO);

    // We'll update edge vertices dynamically in render()
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable point size control in vertex shader
    glEnable(GL_PROGRAM_POINT_SIZE);

    glGenBuffers(1, &m_adjacencySSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_adjacencySSBO);
    std::vector<int> adjacencyMatrix(N * N, 0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            adjacencyMatrix[i * N + j] = m_graph[i].edge_list[j];
        }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, N * N * sizeof(int), adjacencyMatrix.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_adjacencySSBO);

    glGenBuffers(1, &m_degreeSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_degreeSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, N * sizeof(int), m_nodePool, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_degreeSSBO);

    glGenBuffers(1, &m_resultsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_resultsSSBO);
    m_resultsData.resize(N * 2, 0);
    for (int i = 0; i < N; i++) {
        m_resultsData[i * 2 + 0] = 1;
        m_resultsData[i * 2 + 1] = 0;
    }
    m_resultsData[m_startNode * 2 + 0] = 0;
    m_resultsData[m_startNode * 2 + 1] = 1;

    m_resultsData[(m_startNode + 1) * 2 + 0] = 0;
    m_resultsData[(m_startNode + 1) * 2 + 1] = 1;

    m_resultsData[(m_startNode + 4) * 2 + 0] = 0;
    m_resultsData[(m_startNode + 4) * 2 + 1] = 1;
    glBufferData(GL_SHADER_STORAGE_BUFFER, N * 2 * sizeof(float), m_resultsData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_resultsSSBO);

    glGenBuffers(1, &m_lastResultsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lastResultsSSBO);
    m_lastResults.resize(N * 2, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, N * 2 * sizeof(float), m_lastResults.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_lastResultsSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void renderer::getSimulationResults(int startNode, float* susceptibleStates, float* infectedStates) {
    if (startNode < 0 || startNode >= N) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lastResultsSSBO);

    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
        0,
        N * 2 * sizeof(float),
        m_lastResults.data());


    // Extract susceptible and infected states
    for (int i = 0; i < N; i++) {
        susceptibleStates[i] = m_lastResults[i * 2 + 0];
        infectedStates[i] = m_lastResults[i * 2 + 1];
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void renderer::runAllSimulations(float dt) {

    glUseProgram(m_computeID);  // Use COMPUTE program

    GLint loc;
    loc = glGetUniformLocation(m_computeID, "infectionRate");
    glUniform1f(loc, BETA);

	loc = glGetUniformLocation(m_computeID, "alfa");
    glUniform1f(loc, ALFA);

    loc = glGetUniformLocation(m_computeID, "dt");
    glUniform1f(loc, dt);

    loc = glGetUniformLocation(m_computeID, "totalTime");
    glUniform1f(loc, dt);
      
    loc = glGetUniformLocation(m_computeID, "nodeCount");
    glUniform1i(loc, N);

    loc = glGetUniformLocation(m_computeID, "randomSeed");
    glUniform1ui(loc, (unsigned int)(rand()));

    loc = glGetUniformLocation(m_computeID, "startNodeIndex");
    glUniform1i(loc, m_startNode);  

    loc = glGetUniformLocation(m_computeID, "D");
    glUniform1i(loc, D);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    std::swap(m_resultsSSBO, m_lastResultsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_resultsSSBO);  
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_lastResultsSSBO);

    printf("All simulations completed!\n");
}

void renderer::render() {
    float susceptible[N], infected[N];
    getSimulationResults(m_startNode, susceptible, infected);

    for (int i = 0; i < N; i++) {
        m_graph[i].susceptible = susceptible[i];
        m_graph[i].infected = infected[i];
		printf("Node %d: S = %.8f I = %.8f\n", i, susceptible[i], infected[i]);
        //std::cout <<"Node " << i << ": " << "S = " << susceptible[i] << " I = " << infected[i] << std::endl;
    }
    m_shader.use();
    m_shader.setMat4x4("model", m_model);
    m_shader.setMat4x4("view", m_view);
    
    // First, draw edges (lines between connected nodes)
    std::vector<float> edgeVertices;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (m_graph[i].edge_list[j] == 1) {
                // Add line from node i to node j
                edgeVertices.push_back(m_graph[i].x);
                edgeVertices.push_back(m_graph[i].y);
                edgeVertices.push_back(m_graph[j].x);
                edgeVertices.push_back(m_graph[j].y);
            }
        }
    }


    if (!edgeVertices.empty()) {
        glBindVertexArray(m_edgeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_edgeVBO);
        glBufferData(GL_ARRAY_BUFFER, edgeVertices.size() * sizeof(float),
            edgeVertices.data(), GL_DYNAMIC_DRAW);

        m_shader.setInt("isEdge", 1); // Uniform to distinguish edges from nodes
        glDrawArrays(GL_LINES, 0, edgeVertices.size() / 2);
    }

    // Then, draw nodes (points on top of edges)
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_graph.size() * sizeof(Node), m_graph.data(), GL_DYNAMIC_DRAW);

    m_shader.setInt("isEdge", 0); // Uniform to distinguish edges from nodes
    glDrawArrays(GL_POINTS, 0, N); // Draw only N nodes

    glBindVertexArray(0);
}

void renderer::add_node(int i, int j) {
    m_graph[i].edge_list[j] = 1;
    m_graph[j].edge_list[i] = 1;
    m_nodePool[i]++;
    m_nodePool[j]++;
    m_totalDegree += 2;
}

void renderer::generate() {
    for (int i = M0; i < N; i++) {
        int edges_added = 0;
        while (edges_added < M) {
            double p = (double)rand() / RAND_MAX;
            double cumulative = 0.0;
            for (int j = 0; j < i; j++) {
                cumulative += (double)m_nodePool[j] / m_totalDegree;
                if (p < cumulative) {
                    if (m_graph[j].edge_list[i] == 0) {
                        add_node(j, i);
                        edges_added++;
                        break;
                    }
                }
            }
        }
    }
    if (m_adjacencySSBO) {
        std::vector<int> adjacencyMatrix(N * N, 0);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                adjacencyMatrix[i * N + j] = m_graph[i].edge_list[j];
            }
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_adjacencySSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, N * N * sizeof(int), adjacencyMatrix.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    if (m_degreeSSBO) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_degreeSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(int), m_nodePool);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void renderer::setViewMatrix(glm::mat4 matrix) {
    m_view = matrix;
}

int renderer::getInfectedCountForStartNode(int startNode) {
    float susceptible[N], infected[N];
    getSimulationResults(startNode, susceptible, infected);

    int count = 0;
    for (int i = 0; i < N; i++) {
        if (infected[i] == 1) count++;
    }
    return count;
}
void renderer::free() {
    m_graph.clear();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_edgeVAO);
    glDeleteBuffers(1, &m_edgeVBO);
    if (m_lastResultsSSBO) glDeleteBuffers(1, &m_lastResultsSSBO);
    if (m_adjacencySSBO) glDeleteBuffers(1, &m_adjacencySSBO);
    if (m_degreeSSBO) glDeleteBuffers(1, &m_degreeSSBO);
    if (m_resultsSSBO) glDeleteBuffers(1, &m_resultsSSBO);
    resource_manager::clear();
}
