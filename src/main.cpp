//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Computação Gráfica e Visualização I
//               Prof. Eduardo Gastal
//
//     CÓDIGO BASE PARA O TRABALHO FINAL
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <set>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "collisions.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
void DrawVirtualObjectByPattern(const char* pattern, int base_object_id); // Desenha todos os objetos que começam com um padrão
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_material_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;
GLint g_is_shadow_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// Variáveis de posição do jogador
float g_PlayerX = -1.0f;
float g_PlayerY = -1.098f;
float g_PlayerZ = 0.0f;

// Variavel para estado de mira
bool g_AimMode = false;

// Variavel para verificar força do aremesso
float g_RightClickDuration = 0.0f;

// Estados das teclas (false = solta, true = pressionada)
bool g_W_Pressed = false;
bool g_A_Pressed = false;
bool g_S_Pressed = false;
bool g_D_Pressed = false;

// estrutura de instâncias
struct GameObject {
    std::string model_name; 
    int object_id;          // Shader
    glm::vec3 position;     
    glm::vec3 scale;        
    glm::vec3 rotation;     
    bool is_solid;          // true = passível a colisão
    bool is_moving_bezier; // true = objeto move com curva de bézier
    glm::vec3 p0;          // Ponto inicial
    glm::vec3 p1;          // Ponto de controle
    glm::vec3 p2;          // Ponto de de controle 2
    glm::vec3 p3;          // ponto final
    float t_bezier;        // Tempo paramétrico atual
    float speed_bezier;    // Velocidade de deslocamento do t
    bool is_returning;     // Vê se ele está voltando ou não
    bool is_pokebola;       // Identifica se o objeto é uma pokébola ativa
    glm::vec3 launch_dir;   // Direção do arremesso
    float force;            // Força acumulada
    float live_time;        // Contador de tempo (limite de 5 segundos)
    bool hit_ground;        // Se já colidiu e está estática no chão
    bool is_captured = false; // Foi capturado
};
// Vetor de objetos do mapa inteiro
std::vector<GameObject> g_GameWorld;

// Estrutura que mapeia caixas de colisão
struct Collider {
    glm::vec3 min;
    glm::vec3 max;
    int object_id;       
    size_t world_index;
};

// Vetor global que guarda todos os objetos sólidos do cenário
std::vector<Collider> g_SceneColliders;

#define SPHERE           0
#define CHARMANDER       1
#define SQUIRTLE         2
#define PLANE            3
#define CHARMANDER_EYES  4
#define SQUIRTLE_EYES    5
#define TRAINER          6
#define TREE             7
#define GRASS            8
#define POKEBALL         9
#define SKY              10
#define BULBASAUR           11 
#define BULBASAUR_EYES      12 

// Função que coloca todos os objetos no vetor
void InitializeMap() {
    g_GameWorld.clear();

    // 1. O Chão 
    GameObject chao;
    chao.model_name = "the_plane"; 
    chao.object_id  = PLANE;
    chao.position   = glm::vec3(0.0f, -1.1f, 0.0f);
    chao.scale      = glm::vec3(100.0f, 1.0f, 100.0f); 
    chao.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    chao.is_solid   = false;
    chao.is_moving_bezier = false;
    chao.is_pokebola = false;
    chao.is_captured = false;
    g_GameWorld.push_back(chao);
    
    // 2. O Charmander
    GameObject charmander;
    charmander.model_name = "Charmander"; // Padrão que corresponde a todos os shapes do Charmander
    charmander.object_id  = CHARMANDER;
    charmander.position   = glm::vec3(5.0f, -1.05f, 0.0f);
    charmander.scale      = glm::vec3(0.02f, 0.02f, 0.02f);  
    charmander.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    charmander.is_solid   = true;
    charmander.is_moving_bezier = true;
    charmander.p0           = glm::vec3(-10.0f, -1.05f, -5.0f);   
    charmander.p1           = glm::vec3(-5.0f, -1.05f, 5.0f);  
    charmander.p2           = glm::vec3(5.0f, -1.05f, -5.0f);  
    charmander.p3           = glm::vec3(10.0f, -1.05f, 5.0f);  
    charmander.t_bezier     = 0.0f;                          
    charmander.speed_bezier = 0.2f; 
    charmander.is_returning = false; 
    charmander.is_pokebola = false;             
    charmander.is_captured = false;           
    g_GameWorld.push_back(charmander);

    GameObject charmander2;
    charmander2.model_name = "Charmander"; // Padrão que corresponde a todos os shapes do Charmander
    charmander2.object_id  = CHARMANDER;
    charmander2.position   = glm::vec3(5.0f, -1.05f, 0.0f);
    charmander2.scale      = glm::vec3(0.02f, 0.02f, 0.02f);  
    charmander2.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    charmander2.is_solid   = true;
    charmander2.is_moving_bezier = true;
    charmander2.p0           = glm::vec3(5.0f, -1.05f, -10.0f);   
    charmander2.p1           = glm::vec3(0.0f, -1.05f, 0.0f);  
    charmander2.p2           = glm::vec3(-5.0f, -1.05f, 5.0f);  
    charmander2.p3           = glm::vec3(15.0f, -1.05f, 0.0f);  
    charmander2.t_bezier     = 0.0f;                          
    charmander2.speed_bezier = 0.2f; 
    charmander2.is_returning = false; 
    charmander2.is_pokebola = false;             
    charmander2.is_captured = false;           
    g_GameWorld.push_back(charmander2);

    // 3. O Squirtle
    GameObject squirtle;
    squirtle.model_name = "Squirtle"; // Padrão que corresponde a todos os shapes do Squirtle
    squirtle.object_id  = SQUIRTLE;
    squirtle.position   = glm::vec3(15.0f, -1.05f, 0.0f);
    squirtle.scale      = glm::vec3(0.02f, 0.02f, 0.02f);  
    squirtle.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    squirtle.is_solid   = true;
    squirtle.is_moving_bezier = true;
    squirtle.p0           = glm::vec3(15.0f, -1.05f, 0.0f);   
    squirtle.p1           = glm::vec3(15.0f, -1.05f, 5.0f);  
    squirtle.p2           = glm::vec3(20.0f, -1.05f, 5.0f);
    squirtle.p3           = glm::vec3(25.0f, -1.05f, 0.0f);
    squirtle.t_bezier     = 0.0f;                          
    squirtle.speed_bezier = 0.2f;
    squirtle.is_returning = false;
    squirtle.is_pokebola = false;  
    squirtle.is_captured = false;           
    g_GameWorld.push_back(squirtle);

    GameObject bulbasaur;
    bulbasaur.model_name = "Bulbasaur"; 
    bulbasaur.object_id  = BULBASAUR;
    bulbasaur.position   = glm::vec3(15.0f, -1.05f, 0.0f);
    bulbasaur.scale      = glm::vec3(0.02f, 0.02f, 0.02f);  
    bulbasaur.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    bulbasaur.is_solid   = true;
    bulbasaur.is_moving_bezier = true;
    bulbasaur.p0           = glm::vec3(-20.0f, -1.05f, -5.0f);   
    bulbasaur.p1           = glm::vec3(-15.0f, -1.05f, -5.0f);  
    bulbasaur.p2           = glm::vec3(10.0f, -1.05f, 5.0f);
    bulbasaur.p3           = glm::vec3(0.0f, -1.05f, -10.0f);
    bulbasaur.t_bezier     = 0.0f;                          
    bulbasaur.speed_bezier = 0.2f;
    bulbasaur.is_returning = false;
    bulbasaur.is_pokebola = false;  
    bulbasaur.is_captured = false;           
    g_GameWorld.push_back(bulbasaur);


    // 4. Árvores naturais
    float tree_positions[20][2] = {
        { -15.0f,  12.0f }, { -10.0f,  25.0f }, { -8.0f, -10.0f },
        {  0.0f,  18.0f }, {  6.0f,  20.0f }, { 12.0f, -12.0f },
        { 18.0f,  14.0f }, { 22.0f, -5.0f }, { -18.0f, -8.0f },
        { 10.0f, -18.0f }, { -5.0f,  15.0f }, { 5.0f,   25.0f },
        { 15.0f,   5.0f }, { -15.0f, -5.0f }, { -5.0f, -25.0f },
        { 5.0f, -15.0f }, { 15.0f, -25.0f }, { -15.0f, 25.0f },
        { -5.0f,   5.0f }, { 5.0f,   15.0f }
    };

    for (int i = 0; i < 20; ++i)
    {
        GameObject tree;
        tree.model_name = "Arvore";
        tree.object_id  = TREE;
        tree.position   = glm::vec3(tree_positions[i][0], -1.05f, tree_positions[i][1]);
        tree.scale      = glm::vec3(0.6f, 0.6f, 0.6f);
        tree.rotation   = glm::vec3(0.0f, 0.8f * i, 0.0f);
        tree.is_solid   = true;
        tree.is_moving_bezier = false;
        tree.is_pokebola = false;
        squirtle.is_captured = false;
        g_GameWorld.push_back(tree);
    }

    // 5. Capins que permitem ao jogador se esconder sem bloquear movimento.
    const glm::vec3 grass_positions[] = {
        glm::vec3(-14.0f, -1.05f, 11.0f),
        glm::vec3(-9.0f, -1.05f, 24.0f),
        glm::vec3(3.0f, -1.05f, 17.0f),
        glm::vec3(17.0f, -1.05f, -10.0f),
        glm::vec3(-16.0f, -1.05f, -6.0f),
        glm::vec3(-4.0f, -1.05f, -20.0f),
        glm::vec3(0.0f, -1.05f, -15.0f),
        glm::vec3(10.0f, -1.05f, -20.0f),
        glm::vec3(-10.0f, -1.05f, 10.0f),
        glm::vec3(20.0f, -1.05f, 20.0f)
    };

    for (int i = 0; i < 10; ++i)
    {
        GameObject grass;
        grass.model_name = "simple_grass";
        grass.object_id  = GRASS;
        grass.position   = grass_positions[i];
        grass.scale      = glm::vec3(0.60f, 0.60f, 0.60f);
        grass.rotation   = glm::vec3(0.0f, 0.4f * i, 0.0f);
        grass.is_solid   = false;
        grass.is_moving_bezier = false;
        grass.is_pokebola = false;
        squirtle.is_captured = false;
        g_GameWorld.push_back(grass);
    }
    GameObject ceu;
    ceu.model_name = "the_plane"; // Reutilizamos o mesmo .obj do chão
    ceu.object_id  = SKY;
    ceu.position   = glm::vec3(0.0f, 30.0f, 0.0f);    // Alto no mapa
    ceu.scale      = glm::vec3(200.0f, 1.0f, 200.0f); // Tamanho gigante
    // Rotacionamos 180 graus no eixo X (PI radianos) para a face texturizada olhar para baixo
    ceu.rotation   = glm::vec3(3.14159265f, 0.0f, 0.0f); 
    ceu.is_solid   = false;
    ceu.is_moving_bezier = false;
    ceu.is_pokebola = false;
    ceu.is_captured = false;
    g_GameWorld.push_back(ceu);
}

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - 589370 e 588886 - Vicenzo Calleya e Gustavo da Silveira", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

// Carregamento de Texturas
    LoadTextureImage("../../data/rocky_terrain_02_diff_1k.jpg"); // TextureImage1 (Plano)
    LoadTextureImage("../../data/Charmander_BaseColor_1001.png"); // TextureImage2 (Charmander)
    LoadTextureImage("../../data/Squirtle_BaseColor_1001.jpg");   // TextureImage3 (Squirtle)
    LoadTextureImage("../../data/BASE1_Base_Color.png");         // TextureImage4 (Treinador BASE1)
    LoadTextureImage("../../data/BASE2_Base_Color.png");         // TextureImage5 (Treinador BASE2)
    LoadTextureImage("../../data/BASE3_Base_Color.png");         // TextureImage6 (Treinador BASE3)
    LoadTextureImage("../../data/Trunck.jpg");                  // TextureImage7 (Árvore tronco)
    LoadTextureImage("../../data/Leaves1.jpg");                 // TextureImage8 (Árvore folhas 1)
    LoadTextureImage("../../data/Leaves_2_Cartoon.jpg");        // TextureImage9 (Árvore folhas 2)
    LoadTextureImage("../../data/ceu.jpeg");
    LoadTextureImage("../../data/Bulbasaur_BaseColor_1001.jpg");
    // Carregamento de Modelos
    ObjModel treinadormodel("../../data/treinador.obj");
    ComputeNormals(&treinadormodel);
    BuildTrianglesAndAddToVirtualScene(&treinadormodel);
    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel charmandermodel("../../data/Charmander_v01.obj");
    ComputeNormals(&charmandermodel);
    BuildTrianglesAndAddToVirtualScene(&charmandermodel);

    ObjModel squirtlemodel("../../data/Squirtle_v01.obj");
    ComputeNormals(&squirtlemodel);
    BuildTrianglesAndAddToVirtualScene(&squirtlemodel);

    ObjModel bulbasaurmodel("../../data/Bulbasaur_v01.obj"); 
    ComputeNormals(&bulbasaurmodel);                         
    BuildTrianglesAndAddToVirtualScene(&bulbasaurmodel);

    ObjModel treemodel("../../data/Arvore.obj");
    ComputeNormals(&treemodel);
    BuildTrianglesAndAddToVirtualScene(&treemodel);

    ObjModel grassmodel("../../data/simple_grass.obj");
    for (size_t shape = 0; shape < grassmodel.shapes.size(); ++shape)
    {
        std::string base_name = "simple_grass_" + std::to_string(shape);
        if (!grassmodel.shapes[shape].name.empty()) {
            grassmodel.shapes[shape].name = base_name + "_" + grassmodel.shapes[shape].name;
        } else {
            grassmodel.shapes[shape].name = base_name;
        }
    }
    ComputeNormals(&grassmodel);
    BuildTrianglesAndAddToVirtualScene(&grassmodel);

    ObjModel pokeballmodel("../../data/pokeball.obj");
    for (size_t shape = 0; shape < pokeballmodel.shapes.size(); ++shape)
    {
        std::string base_name = "pokeball_" + std::to_string(shape);
        if (!pokeballmodel.shapes[shape].name.empty()) {
            pokeballmodel.shapes[shape].name = base_name + "_" + pokeballmodel.shapes[shape].name;
        } else {
            pokeballmodel.shapes[shape].name = base_name;
        }
    }
    ComputeNormals(&pokeballmodel);
    BuildTrianglesAndAddToVirtualScene(&pokeballmodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Definindo variáveis
    glm::mat4 model = Matrix_Identity();
    glm::vec4 camera_position_c  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 camera_lookat_l    = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 camera_up_vector   = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 camera_view_vector = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    // Variáveis estáticas para controle de tempo
    static float last_frame_time = 0.0f;

    InitializeMap();
    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Cálculo preciso do Delta Time
        float current_time = (float)glfwGetTime();
        float delta_time = current_time - last_frame_time;
        last_frame_time = current_time; 

        // limite de delta_time
        if (delta_time > 0.1f) delta_time = 0.1f;

        // 1. LIMPEZA DO BUFFER (Igual)
        glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(g_GpuProgramID);

        // ========================================================
        // 2. MOVIMENTAÇÃO DO JOGADOR (Calculada primeiro)
        // ========================================================
       float player_speed = 4.0f * delta_time; 
        glm::vec4 move_direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        // Extraímos a direção para onde a CÂMERA está olhando no plano horizontal (X, Z)
        glm::vec4 forward_dir = glm::vec4(camera_view_vector.x, 0.0f, camera_view_vector.z, 0.0f);
        if (norm(forward_dir) > 0.0f) {
            forward_dir = forward_dir / norm(forward_dir);
        }

        glm::vec4 right_dir = crossproduct(forward_dir, camera_up_vector); 
        if (norm(right_dir) > 0.0f) {
            right_dir = right_dir / norm(right_dir);
        }

        if (g_W_Pressed) move_direction += forward_dir;
        if (g_S_Pressed) move_direction -= forward_dir;
        if (g_A_Pressed) move_direction -= right_dir;
        if (g_D_Pressed) move_direction += right_dir;

        // Guarda posição antiga antes de aplicar o movimento
        float oldX = g_PlayerX;
        float oldZ = g_PlayerZ;

        if (norm(move_direction) > 0.0f)
        {
            move_direction = move_direction / norm(move_direction);
            g_PlayerX += move_direction.x * player_speed;
            g_PlayerZ += move_direction.z * player_speed;

            g_AngleY = atan2f(move_direction.x, move_direction.z);
        }

        if (g_AimMode) 
        {
            // O jogador para de girar com o movimento e encara a mira da câmera
            g_AngleY = atan2f(camera_view_vector.x, camera_view_vector.z);
        }

        // Calcula hitbox do jogador, usada depois
        glm::vec3 player_bbox_min = glm::vec3(g_PlayerX - 0.5f, g_PlayerY - 0.5f, g_PlayerZ - 0.5f);
        glm::vec3 player_bbox_max = glm::vec3(g_PlayerX + 0.5f, g_PlayerY + 0.5f, g_PlayerZ + 0.5f);

        // ========================================================
        // 3. ATUALIZAÇÃO DA CURVA DE BÉZIER DOS POKÉMONS
        // ========================================================
        for (auto& obj : g_GameWorld)
        {
            if (obj.is_moving_bezier)
            {
                glm::vec3 old_obj_pos = obj.position;
                float old_t = obj.t_bezier;

                if (!obj.is_returning) {
                    obj.t_bezier += obj.speed_bezier * delta_time;
                    if (obj.t_bezier >= 1.0f) {
                        obj.t_bezier = 1.0f;
                        obj.is_returning = true; 
                    }
                } else {
                    obj.t_bezier -= obj.speed_bezier * delta_time;
                    if (obj.t_bezier <= 0.0f) {
                        obj.t_bezier = 0.0f;
                        obj.is_returning = false; 
                    }
                }

                float t = obj.t_bezier;
                float inv_t  = 1.0f - t;

                float b0 = inv_t * inv_t * inv_t;
                float b1 = 3.0f * inv_t * inv_t * t;
                float b2 = 3.0f * inv_t * t * t;  
                float b3 = t * t * t;

                //fórmula de Bézier
                obj.position = b0 * obj.p0 + b1 * obj.p1 + b2 * obj.p2 + b3 * obj.p3;

                // Hitbox temporária para objeto
                glm::vec3 obj_min = g_VirtualScene[obj.model_name].bbox_min + obj.position;
                glm::vec3 obj_max = g_VirtualScene[obj.model_name].bbox_max + obj.position;

                if (CheckCollision_AABB(player_bbox_min, player_bbox_max, obj_min, obj_max)) 
                {
                    obj.position = old_obj_pos;
                    obj.t_bezier = old_t;
                }
                else // cálculo de rotação para ele ficar de frente com o caminho que está andando
                {
                    glm::vec3 direction = 3.0f * inv_t * inv_t * (obj.p1 - obj.p0) + 6.0f * inv_t * t * (obj.p2 - obj.p1) + 3.0f * t * t * (obj.p3 - obj.p2);
                    
                    if (obj.is_returning) {
                        direction = -direction;
                    }

                    obj.rotation.y = atan2(direction.x, direction.z);
                }
            }
        }

        // ========================================================
        // 4. ACÚMULO DE FORÇA E LANÇAMENTO DA POKÉBOLA
        // ========================================================
            if (g_AimMode && g_RightMouseButtonPressed) 
        {
            // Enquanto segurar o botão direito no modo de mira, acumula tempo/força
            g_RightClickDuration += delta_time;
            // Limitador de força máxima
            if (g_RightClickDuration > 2.0f) g_RightClickDuration = 2.0f;
        }
        
        else if (g_AimMode && g_RightClickDuration > 0.0f) 
        {
            // Criamos uma Pokébola
            GameObject pokebola;
            pokebola.model_name = "pokeball";
            pokebola.object_id  = POKEBALL;
            
            // Nasce um pouco à frente do peito do jogador para não colidir com ele mesmo
            pokebola.position   = glm::vec3(g_PlayerX, g_PlayerY + 1.5f, g_PlayerZ + 0.5f) 
                                + (glm::vec3(camera_view_vector.x, 0.0f, camera_view_vector.z) * 0.3f);
            
            pokebola.scale      = glm::vec3(0.1f, 0.1f, 0.1f);
            pokebola.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
            pokebola.is_solid   = false;

            glm::vec4 raw_dir = glm::vec4(camera_view_vector.x, camera_view_vector.y, camera_view_vector.z, 0.0f);
            float n = norm(raw_dir);

            if (n > 0.0f) {
                raw_dir = raw_dir / n;
            }
            
            // Atributos específicos do arremesso:
            pokebola.is_pokebola       = true;
            pokebola.is_moving_bezier  = false;
            pokebola.launch_dir        = glm::vec3(raw_dir.x, raw_dir.y, raw_dir.z);
            pokebola.force             = g_RightClickDuration; 
            pokebola.live_time         = 0.0f;                 // Começa o relógio de 5s
            pokebola.hit_ground        = false;                // Está voando
            g_GameWorld.push_back(pokebola);

            // Reseta o acumulador para o próximo tiro
            g_RightClickDuration = 0.0f;
        }

        for (auto& obj : g_GameWorld) 
        {
            if (obj.is_pokebola) 
            {
                obj.live_time += delta_time; 

                if (!obj.hit_ground) 
                {
                    float gravity = 9.8f; 
                    float launch_speed = obj.force * 20.0f; 

                    obj.position.x += obj.launch_dir.x * launch_speed * delta_time;
                    obj.position.z += obj.launch_dir.z * launch_speed * delta_time;

                    float initial_vy = obj.launch_dir.y * launch_speed;
                    float current_vy = initial_vy - (gravity * obj.live_time);
                    obj.position.y += current_vy * delta_time;

                    obj.rotation.x += 5.0f * delta_time;

                    // Colisão com o chão estável
                    float pokeball_radius = 1.0f * obj.scale.y;
                    float ground_y = -1.0f + pokeball_radius;
                    if (obj.position.y <= ground_y) 
                    {
                        obj.position.y = ground_y;
                        obj.hit_ground = true;
                        obj.rotation.x = 0.0f;
                    }

                    // TESTE DE COLISÃO DA POKÉBOLA USANDO OS COLLIDERS DO MAPA
                    for (const auto& collider : g_SceneColliders)
                    {
                        // Pegamos o centro aproximado do objeto alvo
                        glm::vec3 target_center = (collider.min + collider.max) * 0.5f;

                        // Calculamos a distância horizontal (X e Z) entre a pokébola e o objeto
                        float dx = obj.position.x - target_center.x;
                        float dz = obj.position.z - target_center.z;
                        float distance_xz = sqrt(dx * dx + dz * dz);

                        if (distance_xz < 0.75f && std::abs(obj.position.y - target_center.y) < 3.0f)
                        {
                            // 1. SE FOR POKÉMON -> CAPTURA
                            if (collider.object_id == CHARMANDER || collider.object_id == SQUIRTLE)
                            {
                                g_GameWorld[collider.world_index].is_moving_bezier = false;
                                g_GameWorld[collider.world_index].is_solid = false;
                                g_GameWorld[collider.world_index].position.y = -10.0f; // Some com ele
                                g_GameWorld[collider.world_index].is_captured = true;
                                
                                float pokeball_radius = 1.0f * obj.scale.y;
                                float ground_y = -1.0f + pokeball_radius;
                                obj.hit_ground = true;
                                obj.is_captured = true;
                                obj.position.y = ground_y;
                                obj.rotation.x = 0.0f;
                                break;
                            }
                            // 2. SE FOR OUTRA COISA SÓLIDA -> QUICA
                            else
                            {
                                // Inversão simples de trajetória baseada de onde ela veio
                                if (std::abs(dx) > std::abs(dz)) {
                                    obj.launch_dir.x = -obj.launch_dir.x;
                                } else {
                                    obj.launch_dir.z = -obj.launch_dir.z;
                                }

                                obj.force *= 0.6f;
                                if (obj.force < 0.4f) {
                                    float pokeball_radius = 1.0f * obj.scale.y;
                                    float ground_y = -1.0f + pokeball_radius;
                                    obj.hit_ground = true;
                                    obj.position.y = ground_y;
                                    obj.rotation.x = 0.0f;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }   

        // ========================================================
        // 5. TESTE DE COLISÃO
        // ========================================================
        
        g_SceneColliders.clear();

        // 1. Constrói as caixas de colisão de tudo que é sólido no mundo
        for (size_t i = 0; i < g_GameWorld.size(); ++i) 
        {
            // Se for uma pokébola ativa voando, ela não bloqueia o jogador
            if (g_GameWorld[i].is_pokebola && !g_GameWorld[i].hit_ground) continue;
            if (g_GameWorld[i].is_captured) continue;
            if (g_GameWorld[i].is_solid) 
            {
                Collider c;
                c.min = g_VirtualScene[g_GameWorld[i].model_name].bbox_min + g_GameWorld[i].position;
                c.max = g_VirtualScene[g_GameWorld[i].model_name].bbox_max + g_GameWorld[i].position;
                c.object_id = g_GameWorld[i].object_id;
                c.world_index = i; // Guarda a posição para a pokébola poder capturar
                
                g_SceneColliders.push_back(c);
            }
        }

        // 2. Verifica as colisões do JOGADOR contra a lista global
        for (const auto& collider : g_SceneColliders) 
        {
            if (CheckCollision_AABB(player_bbox_min, player_bbox_max, collider.min, collider.max)) 
            {
                g_PlayerX = oldX;
                g_PlayerZ = oldZ;
                break;
            }
        }

        // ========================================================
        // 6. CLEANUP DE POKEBOLAS
        // ========================================================

        g_GameWorld.erase(
        std::remove_if(g_GameWorld.begin(), g_GameWorld.end(),
            [](const GameObject& obj) {
                return obj.is_pokebola && obj.live_time > 5.0f;
            }),
        g_GameWorld.end()
        );
        
        // ========================================================
        // 7. ATUALIZAÇÃO DA CÂMERA (Usa a posição confirmada do jogador)
        // ========================================================
        glm::vec4 player_pos = glm::vec4(g_PlayerX, g_PlayerY, g_PlayerZ, 1.0f);

        glm::vec4 camera_lookat_l = player_pos;

        float r = g_CameraDistance;
        if (g_AimMode){
            r = g_CameraDistance * 0.7f; // Zoom aproximado de mira
            camera_lookat_l += glm::vec4(0.0f, 1.5f, 0.5f, 0.0f);
        }
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        glm::vec4 base_camera_pos = camera_lookat_l + glm::vec4(x, y, z, 0.0f);

        if (g_AimMode) 
        {
            glm::vec4 temporary_view = camera_lookat_l - base_camera_pos;
            glm::vec4 right_dir = crossproduct(temporary_view, camera_up_vector);
            if (norm(right_dir) > 0.0f) {
                right_dir = right_dir / norm(right_dir);
            }
            // Desloca a posição da câmera e o ponto de olhar um pouco para a direita
            base_camera_pos += right_dir * 0.5f; 
            player_pos += right_dir * 0.5f; 
        }

        camera_position_c  = player_pos + glm::vec4(x, y, z, 0.0f);
        if (camera_position_c.y < -1.0f) 
        {
            camera_position_c.y = -1.0f;
        } 
        camera_view_vector = camera_lookat_l - camera_position_c; 
        camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); 

        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Matriz de Projeção (Perspectiva ou Ortográfica)
        glm::mat4 projection;
        float nearplane = -0.1f;  
        float farplane  = -100.0f; 

        if (g_UsePerspectiveProjection) {
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        } else {
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r_ortho = t*g_ScreenRatio;
            float l = -r_ortho;
            projection = Matrix_Orthographic(l, r_ortho, b, t, nearplane, farplane);
        }

        // Enviamos as matrizes globais de visualização atualizadas para a GPU
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // ========================================================
        // 8. DESENHO DOS MODELOS (Com as posições e matrizes 100% corrigidas)
        // ========================================================

        // Jogador humano controlável
        model = Matrix_Translate(g_PlayerX, g_PlayerY, g_PlayerZ)
              * Matrix_Rotate_Y(g_AngleY)
              * Matrix_Scale(0.3f, 0.3f, 0.3f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, TRAINER);
        DrawVirtualObjectByPattern("BASE", TRAINER);

        // Todos os outros objetos
        for (const auto& obj : g_GameWorld)
        {
            if (obj.is_captured && !obj.is_pokebola) continue;
            model = Matrix_Translate(obj.position.x, obj.position.y, obj.position.z)
                  * Matrix_Rotate_Y(obj.rotation.y)
                  * Matrix_Rotate_X(obj.rotation.x)
                  * Matrix_Rotate_Z(obj.rotation.z)
                  * Matrix_Scale(obj.scale.x, obj.scale.y, obj.scale.z);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, obj.object_id);
            DrawVirtualObjectByPattern(obj.model_name.c_str(), obj.object_id);
        }
// ========================================================
        // PREPARAÇÃO PARA O SISTEMA DE SOMBRAS
        // ========================================================
        // 1. Encontra a Pokébola (se existir e estiver no chão)
        bool has_active_pokeball = false;
        glm::vec3 active_pokeball_pos(0.0f);
        for (const auto& obj : g_GameWorld) {
            if (obj.is_pokebola && obj.hit_ground && obj.is_captured) {
                has_active_pokeball = true;
                active_pokeball_pos = obj.position;
                break; // Usa a primeira que encontrar
            }
        }
        float pb_radius = 6.0f; // Tamanho da área afetada

        // Envia dados para o Shader
        glUniform3f(glGetUniformLocation(g_GpuProgramID, "pokeball_pos"), active_pokeball_pos.x, active_pokeball_pos.y, active_pokeball_pos.z);
        glUniform1f(glGetUniformLocation(g_GpuProgramID, "pokeball_radius"), has_active_pokeball ? pb_radius : 0.0f);

        glm::vec4 light_dir = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f); 
        float ground_y = -1.095f; 

        glUniform1i(g_is_shadow_uniform, 1); 
        glDisable(GL_CULL_FACE); 

        // ========================================================
        // PASSO A: SOMBRAS GLOBAIS (O sol)
        // ========================================================
        glUniform1i(glGetUniformLocation(g_GpuProgramID, "shadow_type"), 0);

        glm::mat4 S = Matrix_Identity();
        if (light_dir.y != 0.0f) {
            S[1][0] = -light_dir.x / light_dir.y;
            S[1][1] = 0.0f;
            S[1][2] = -light_dir.z / light_dir.y;
            S[3][0] = ground_y * (light_dir.x / light_dir.y);
            S[3][1] = ground_y;
            S[3][2] = ground_y * (light_dir.z / light_dir.y);
        }

        // Sombra global do treinador 
        // Correção de Bug do seu código antigo: Trocado 'ground_y' por 'g_PlayerY'.
        // Isso faz com que a sombra estique em vez de ficar como um bloco nos pés dele.
        glm::mat4 player_model = Matrix_Translate(g_PlayerX, g_PlayerY, g_PlayerZ)
                               * Matrix_Rotate_Y(g_AngleY)
                               * Matrix_Scale(0.3f, 0.3f, 0.3f);
        glm::mat4 player_shadow = S * player_model;
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(player_shadow));
        glUniform1i(g_object_id_uniform, TRAINER);
        DrawVirtualObjectByPattern("BASE", TRAINER);

        // Sombra global dos objetos
        for (const auto& obj : g_GameWorld) {
            if (obj.object_id == PLANE || obj.object_id == GRASS || obj.object_id == SKY) continue;
            if (obj.is_captured && !obj.is_pokebola) continue;
            glm::mat4 obj_model = Matrix_Translate(obj.position.x, obj.position.y, obj.position.z)
                                * Matrix_Rotate_Y(obj.rotation.y)
                                * Matrix_Rotate_X(obj.rotation.x)
                                * Matrix_Rotate_Z(obj.rotation.z)
                                * Matrix_Scale(obj.scale.x, obj.scale.y, obj.scale.z);

            glm::mat4 obj_shadow = S * obj_model;
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(obj_shadow));
            glUniform1i(g_object_id_uniform, obj.object_id);
            DrawVirtualObjectByPattern(obj.model_name.c_str(), obj.object_id);
        }

        // ========================================================
        // PASSO B: SOMBRAS RADIAIS (A Pokébola)
        // ========================================================
        if (has_active_pokeball) {
            glUniform1i(glGetUniformLocation(g_GpuProgramID, "shadow_type"), 1);

            // Simulamos uma fonte de luz levemente acima do chão para não criar divisões por 0
            // E garantir que a sombra tenha um limite de esticamento.
            glm::vec3 pb_light_pos = active_pokeball_pos + glm::vec3(0.0f, 2.0f, 0.0f); 

            // 1. Sombra radial do treinador
            // (Se ele estiver muito longe, ignoramos para economizar processamento)
            if (glm::distance(glm::vec2(g_PlayerX, g_PlayerZ), glm::vec2(pb_light_pos.x, pb_light_pos.z)) < pb_radius * 1.5f) {
                glm::vec4 dir_to_light = glm::vec4(pb_light_pos.x - g_PlayerX, pb_light_pos.y - g_PlayerY, pb_light_pos.z - g_PlayerZ, 0.0f);
                glm::mat4 S_rad = Matrix_Identity();
                if (dir_to_light.y != 0.0f) {
                    S_rad[1][0] = -dir_to_light.x / dir_to_light.y;
                    S_rad[1][1] = 0.0f;
                    S_rad[1][2] = -dir_to_light.z / dir_to_light.y;
                    S_rad[3][0] = ground_y * (dir_to_light.x / dir_to_light.y);
                    S_rad[3][1] = ground_y;
                    S_rad[3][2] = ground_y * (dir_to_light.z / dir_to_light.y);
                }
                player_shadow = S_rad * player_model;
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(player_shadow));
                glUniform1i(g_object_id_uniform, TRAINER);
                DrawVirtualObjectByPattern("BASE", TRAINER);
            }

            // 2. Sombra radial dos objetos
            for (const auto& obj : g_GameWorld) {
                if (obj.object_id == PLANE || obj.object_id == GRASS || obj.is_pokebola || obj.object_id == SKY) continue;
                if (obj.is_captured && !obj.is_pokebola) continue;
                // Só desenha se estiver próximo da luz da pokébola (otimização)
                if (glm::distance(glm::vec2(obj.position.x, obj.position.z), glm::vec2(pb_light_pos.x, pb_light_pos.z)) > pb_radius * 1.5f) continue;

                // O macete mágico: O vetor aponta do objeto PARA a pokebola!
                glm::vec4 dir_to_light = glm::vec4(pb_light_pos.x - obj.position.x, pb_light_pos.y - obj.position.y, pb_light_pos.z - obj.position.z, 0.0f);
                
                glm::mat4 S_rad_obj = Matrix_Identity();
                if (dir_to_light.y != 0.0f) {
                    S_rad_obj[1][0] = -dir_to_light.x / dir_to_light.y;
                    S_rad_obj[1][1] = 0.0f;
                    S_rad_obj[1][2] = -dir_to_light.z / dir_to_light.y;
                    S_rad_obj[3][0] = ground_y * (dir_to_light.x / dir_to_light.y);
                    S_rad_obj[3][1] = ground_y;
                    S_rad_obj[3][2] = ground_y * (dir_to_light.z / dir_to_light.y);
                }

                glm::mat4 obj_model = Matrix_Translate(obj.position.x, obj.position.y, obj.position.z)
                                    * Matrix_Rotate_Y(obj.rotation.y)
                                    * Matrix_Rotate_X(obj.rotation.x)
                                    * Matrix_Rotate_Z(obj.rotation.z)
                                    * Matrix_Scale(obj.scale.x, obj.scale.y, obj.scale.z);

                glm::mat4 obj_shadow = S_rad_obj * obj_model;
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(obj_shadow));
                glUniform1i(g_object_id_uniform, obj.object_id);
                DrawVirtualObjectByPattern(obj.model_name.c_str(), obj.object_id);
            }
        }

        glEnable(GL_CULL_FACE);
        glUniform1i(g_is_shadow_uniform, 0);
        
        // RENDERING DE TEXTO E SWAP BUFFERS (Igual)
        TextRendering_ShowEulerAngles(window);
        TextRendering_ShowProjection(window);
        TextRendering_ShowFramesPerSecond(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glUniform1i(g_material_id_uniform, 0);
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função auxiliar que desenha todos os objetos que contenham um padrão no nome
// Útil para desenhar modelos com múltiplos shapes (cabeça, corpo, mãos, etc.)
// O padrão "Charmander" vai desenhar todos os shapes que contenham "Charmander" no nome
void DrawVirtualObjectByPattern(const char* pattern, int base_object_id)
{
    bool found = false;
    std::string pattern_str(pattern);
    bool was_cull_enabled = glIsEnabled(GL_CULL_FACE);
    if (base_object_id == POKEBALL) {
        glDisable(GL_CULL_FACE);
    }
    
    // Tenta desenhar todos os objetos que contenham o padrão no nome
    for (const auto& pair : g_VirtualScene) {
        const std::string& name = pair.first;
        // Verifica se o nome contém o padrão como palavra-chave
        bool matches_pattern = (name.find(pattern_str) != std::string::npos);
        if (!matches_pattern && pattern_str == "Arvore") {
            matches_pattern = (name == "Vert" || name.find("Cube") != std::string::npos);
        }

        if (matches_pattern) {
            found = true;
            
            // Detecta se é um olho e ajusta o object_id
            int object_id_to_use = base_object_id;
            int material_id_to_use = 0;

            if (name.find("eyes") != std::string::npos || name.find("eye") != std::string::npos) {
                if (pattern_str.find("Charmander") != std::string::npos) {
                    object_id_to_use = CHARMANDER_EYES;
                } else if (pattern_str.find("Squirtle") != std::string::npos) {
                    object_id_to_use = SQUIRTLE_EYES;
                } else if (pattern_str.find("Bulbasaur") != std::string::npos) { 
                    object_id_to_use = BULBASAUR_EYES;
                }
            }
            else if (name.find("BASE1") != std::string::npos) {
                material_id_to_use = 1;
            }
            else if (name.find("BASE2") != std::string::npos) {
                material_id_to_use = 2;
            }
            else if (name.find("BASE3") != std::string::npos) {
                material_id_to_use = 3;
            }
            else if (pattern_str == "pokeball") {
                if (name.find("pokeball_0") != std::string::npos) {
                    material_id_to_use = 1;
                }
                else if (name.find("pokeball_1") != std::string::npos) {
                    material_id_to_use = 2;
                }
                else if (name.find("pokeball_2") != std::string::npos) {
                    material_id_to_use = 3;
                }
            }
            else if (pattern_str == "Arvore") {
                object_id_to_use = TREE;
                if (name == "Vert") {
                    material_id_to_use = 3;
                }
                else if (name == "Cube.007" || name == "Cube.006" || name == "Cube.003") {
                    material_id_to_use = 2;
                }
                else if (name.find("Cube") != std::string::npos) {
                    material_id_to_use = 1;
                }
            }
            
            glUniform1i(g_object_id_uniform, object_id_to_use);
            glUniform1i(g_material_id_uniform, material_id_to_use);
            
            glBindVertexArray(pair.second.vertex_array_object_id);
            
            glm::vec3 bbox_min = pair.second.bbox_min;
            glm::vec3 bbox_max = pair.second.bbox_max;
            glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
            glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
            
            glDrawElements(
                pair.second.rendering_mode,
                pair.second.num_indices,
                GL_UNSIGNED_INT,
                (void*)(pair.second.first_index * sizeof(GLuint))
            );
        }
    }
    
    glBindVertexArray(0);

    if (base_object_id == POKEBALL && was_cull_enabled) {
        glEnable(GL_CULL_FACE);
    }
    
    // Se nenhum encontrado com o padrão, tenta com o nome exato (para compatibilidade)
    if (!found && g_VirtualScene.find(pattern_str) != g_VirtualScene.end()) {
        DrawVirtualObject(pattern_str.c_str());
    }
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_material_id_uniform = glGetUniformLocation(g_GpuProgramID, "material_id");
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");
    g_is_shadow_uniform  = glGetUniformLocation(g_GpuProgramID, "is_shadow");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage6"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage7"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage8"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage9"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage10"), 10);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".

    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        assert(model->shapes[shape].mesh.smoothing_group_ids.size() == num_triangles);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            assert(sgroup >= 0);
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );

    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];

                const glm::vec4  n = crossproduct(b-a,c-a);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);

        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);

            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;
    
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;
    
        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;
    
        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.3f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
    if (g_CameraDistance >= 10)
        g_CameraDistance = 10;
}



void Correcao_KeyCallback(int key, int action, int mod);

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =======================
    // Não modifique esta chamada! Ela é utilizada para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    Correcao_KeyCallback(key, action, mod);
    // =======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    // Controles de movimentação do jogador (W, A, S e D)

    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)   g_W_Pressed = true;
        if (action == GLFW_RELEASE) g_W_Pressed = false;
    }
    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)   g_S_Pressed = true;
        if (action == GLFW_RELEASE) g_S_Pressed = false;
    }
    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)   g_A_Pressed = true;
        if (action == GLFW_RELEASE) g_A_Pressed = false;
    }
    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)   g_D_Pressed = true;
        if (action == GLFW_RELEASE) g_D_Pressed = false;
    }

    // Controles de mira (E)
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
        g_AimMode = !g_AimMode;
    }
    
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

