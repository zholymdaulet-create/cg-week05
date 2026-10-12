// 5-апта: индекс буфері (EBO), примитив түрлері
// Базалық: 4 вершиналы түрлі-түсті төртбұрыш, EBO + glDrawElements, шеңбер бойымен қозғалады (uniform uOffset)
// Пернелер: 1 = GL_TRIANGLES, 2 = GL_LINE_LOOP, 3 = GL_LINE_STRIP, 4 = GL_POINTS, Tab = wireframe, Esc = шығу

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;

// ===== Тапсырма ауыстырғыштары =====
const bool TASK1_BROKEN_INDICES = false;  // 1: индекс {0,1,2}, саны 3 — бір ғана үшбұрыш
const bool TASK2_PENTAGON       = false;  // 2: бесбұрыш (5 вершина, 9 индекс)
const bool TASK3_TWO_OBJECTS    = true;   // 3: екі объект (+ қосымша: қарама-қарсы бағытта қозғалады)

// ===== Жаһандық күй =====
GLenum g_drawMode      = GL_TRIANGLES;
bool   g_wireframe     = false;
bool   g_tabWasPressed = false;

const char* VERTEX_SHADER_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform vec2 uOffset;          // 4-аптадағы uniform — объектіні жылжытады
out vec3 vColor;
void main()
{
    gl_Position = vec4(aPos.xy + uOffset, aPos.z, 1.0);
    vColor = aColor;
}
)";

const char* FRAGMENT_SHADER_SRC = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";

void onResize(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Қосымша: примитив түрлерін сынау
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) g_drawMode = GL_TRIANGLES;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) g_drawMode = GL_LINE_LOOP;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) g_drawMode = GL_LINE_STRIP;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) g_drawMode = GL_POINTS;

    // Қосымша: Tab — wireframe (тек басылған сәтте ауысады)
    bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabPressed && !g_tabWasPressed)
        g_wireframe = !g_wireframe;
    g_tabWasPressed = tabPressed;
}

GLuint compileShader(GLenum type, const char* source)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(id, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return id;
}

GLuint makeShader(const char* vertexSrc, const char* fragmentSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    int ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "cg", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }
    std::cout << "OpenGL:   " << glGetString(GL_VERSION)  << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";

    GLuint shaderProgram = makeShader(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    GLint  locOffset     = glGetUniformLocation(shaderProgram, "uOffset");

    // ===== Төртбұрыш: 4 вершина (6 емес), позиция + түс =====
    float quadVertices[] = {
         0.3f,  0.3f, 0.0f,   1.0f, 0.0f, 0.0f,   // 0 — оң жоғарғы
         0.3f, -0.3f, 0.0f,   0.0f, 1.0f, 0.0f,   // 1 — оң төменгі
        -0.3f, -0.3f, 0.0f,   0.0f, 0.0f, 1.0f,   // 2 — сол төменгі
        -0.3f,  0.3f, 0.0f,   1.0f, 1.0f, 0.0f    // 3 — сол жоғарғы
    };
    unsigned int quadIndices[] = {
        0, 1, 3,   // бірінші үшбұрыш
        1, 2, 3    // екінші үшбұрыш
    };
    // 1-тапсырма: индекстерді бұзу
    unsigned int brokenIndices[] = { 0, 1, 2 };

    // ===== 2-тапсырма: бесбұрыш, 5 вершина =====
    // x = cos(бұрыш) * 0.3, y = sin(бұрыш) * 0.3; бұрыштар: −90°, −18°, 54°, 126°, 198°
    float pentagonVertices[5 * 6];
    const float pentagonAnglesDeg[5]  = { -90.0f, -18.0f, 54.0f, 126.0f, 198.0f };
    const float pentagonColors[5][3]  = { {1,0,0}, {1,0.6f,0}, {1,1,0}, {0,1,0}, {0,0.5f,1} };
    for (int i = 0; i < 5; ++i) {
        float angleRad = pentagonAnglesDeg[i] * 3.14159265f / 180.0f;
        pentagonVertices[i * 6 + 0] = std::cos(angleRad) * 0.3f;
        pentagonVertices[i * 6 + 1] = std::sin(angleRad) * 0.3f;
        pentagonVertices[i * 6 + 2] = 0.0f;
        pentagonVertices[i * 6 + 3] = pentagonColors[i][0];
        pentagonVertices[i * 6 + 4] = pentagonColors[i][1];
        pentagonVertices[i * 6 + 5] = pentagonColors[i][2];
    }
    // 0-вершина — барлық үшбұрышқа ортақ «орталық» нүкте: 3 үшбұрыш, 9 индекс
    unsigned int pentagonIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4
    };

    // ===== Қай геометрияны жүктейміз =====
    const float*        vertexData;
    GLsizeiptr          vertexBytes;
    const unsigned int* indexData;
    GLsizeiptr          indexBytes;
    GLsizei             indexCount;   // glDrawElements-ке — ИНДЕКС саны, вершина саны емес

    if (TASK2_PENTAGON) {
        vertexData = pentagonVertices; vertexBytes = sizeof(pentagonVertices);
        indexData  = pentagonIndices;  indexBytes  = sizeof(pentagonIndices);  indexCount = 9;
    } else if (TASK1_BROKEN_INDICES) {
        vertexData = quadVertices;     vertexBytes = sizeof(quadVertices);
        indexData  = brokenIndices;    indexBytes  = sizeof(brokenIndices);    indexCount = 3;
    } else {
        vertexData = quadVertices;     vertexBytes = sizeof(quadVertices);
        indexData  = quadIndices;      indexBytes  = sizeof(quadIndices);      indexCount = 6;
    }

    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);   // VAO бірінші — EBO байланысы VAO ішінде сақталады

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertexData, GL_STATIC_DRAW);

    // EBO VAO байланып тұрғанда жасалады
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, indexData, GL_STATIC_DRAW);

    const int STRIDE = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) ӘДЕЙІ жазылмаған — VAO-дан EBO өшіп қалуы мүмкін

    glPointSize(10.0f);   // қосымша: GL_POINTS режимінде нүктелер көрінетін болсын

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        float t = (float)glfwGetTime();

        if (TASK3_TWO_OBJECTS) {
            // 3-тапсырма: бір VAO, екі glUniform2f + glDrawElements жұбы.
            // Қосымша: екеуі қарама-қарсы бағытта айналады (t және −t), бір-біріне тимейді
            const float orbitRadius = 0.15f;
            glUniform2f(locOffset, -0.5f + std::cos(t) * orbitRadius,  std::sin(t) * orbitRadius);
            glDrawElements(g_drawMode, indexCount, GL_UNSIGNED_INT, 0);

            glUniform2f(locOffset,  0.5f + std::cos(-t) * orbitRadius, std::sin(-t) * orbitRadius);
            glDrawElements(g_drawMode, indexCount, GL_UNSIGNED_INT, 0);
        } else {
            // Базалық: шеңбер бойымен қозғалыс (0.3 + 0.5 = 0.8 < 1, NDC-ден шықпайды)
            const float orbitRadius = 0.5f;
            glUniform2f(locOffset, std::cos(t) * orbitRadius, std::sin(t) * orbitRadius);
            glDrawElements(g_drawMode, indexCount, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
