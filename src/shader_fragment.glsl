#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE              0
#define CHARMANDER          1
#define SQUIRTLE            2
#define PLANE               3
#define CHARMANDER_EYES     4
#define SQUIRTLE_EYES       5
#define TRAINER             6
#define TREE                7
#define GRASS               8
uniform int object_id;
uniform int material_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;
uniform sampler2D TextureImage6;
uniform sampler2D TextureImage7;
uniform sampler2D TextureImage8;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Coeficiente de refletância difusa
    vec3 Kd0;

    if ( object_id == SPHERE )
    {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 d = position_model - bbox_center;

        float rho   = length(d);
        float theta = atan(d.x,d.z);
        float phi   = asin(d.y / rho);

        U = (theta + M_PI) / 2.0 / M_PI;
        V = (phi + M_PI_2) / M_PI;

        Kd0 = texture(TextureImage0, vec2(U,V)).rgb;
    }
    else if ( object_id == CHARMANDER )
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureImage1, vec2(U,V)).rgb;
    }
    else if ( object_id == SQUIRTLE )
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureImage2, vec2(U,V)).rgb;
    }
    else if ( object_id == TRAINER )
    {
        U = texcoords.x;
        V = texcoords.y;
        if (material_id == 1)
            Kd0 = texture(TextureImage3, vec2(U,V)).rgb;
        else if (material_id == 2)
            Kd0 = texture(TextureImage4, vec2(U,V)).rgb;
        else if (material_id == 3)
            Kd0 = texture(TextureImage5, vec2(U,V)).rgb;
        else
            Kd0 = texture(TextureImage3, vec2(U,V)).rgb;
    }
    else if ( object_id == TREE )
    {
        U = texcoords.x;
        V = texcoords.y;
        if (material_id == 3)
            Kd0 = texture(TextureImage6, vec2(U,V)).rgb;
        else if (material_id == 2)
            Kd0 = texture(TextureImage8, vec2(U,V)).rgb;
        else
            Kd0 = texture(TextureImage7, vec2(U,V)).rgb;
    }
    else if ( object_id == GRASS )
    {
        U = texcoords.x;
        V = texcoords.y;
        Kd0 = texture(TextureImage8, vec2(U,V)).rgb;
    }
    else if ( object_id == PLANE )
    {
        U = texcoords.x * 100.0f;
        V = texcoords.y * 100.0f;
        Kd0 = texture(TextureImage0, vec2(U,V)).rgb;
    }
    else if ( object_id == CHARMANDER_EYES )
    {
        // Os olhos do Charmander: pupila + iris
        // Normalizar position_model pela bbox para obter coordenadas 0-1
        vec3 normalized_pos = (position_model.xyz - bbox_min.xyz) / (bbox_max.xyz - bbox_min.xyz);
        vec2 eye_center = vec2(0.5, 0.5);
        float dist_from_center = length(normalized_pos.xy - eye_center);
        
        if (dist_from_center < 0.08) {
            // Pupila (preta bem escura)
            Kd0 = vec3(0.02, 0.02, 0.02);
        } else if (dist_from_center < 0.3) {
            // Iris (branca/clara)
            Kd0 = mix(vec3(0.95, 0.95, 0.95), vec3(0.1, 0.1, 0.1), (dist_from_center - 0.08) / 0.22);
        } else {
            // Branco do olho (levemente cinzento)
            Kd0 = vec3(0.85, 0.85, 0.85);
        }
    }
    else if ( object_id == SQUIRTLE_EYES )
    {
        // Os olhos do Squirtle: pupila + iris
        // Normalizar position_model pela bbox para obter coordenadas 0-1
        vec3 normalized_pos = (position_model.xyz - bbox_min.xyz) / (bbox_max.xyz - bbox_min.xyz);
        vec2 eye_center = vec2(0.5, 0.5);
        float dist_from_center = length(normalized_pos.xy - eye_center);
        
        if (dist_from_center < 0.08) {
            // Pupila (preta bem escura)
            Kd0 = vec3(0.02, 0.02, 0.02);
        } else if (dist_from_center < 0.3) {
            // Iris (branca/clara)
            Kd0 = mix(vec3(0.95, 0.95, 0.95), vec3(0.1, 0.1, 0.1), (dist_from_center - 0.08) / 0.22);
        } else {
            // Branco do olho (levemente cinzento)
            Kd0 = vec3(0.85, 0.85, 0.85);
        }
    }

    // Equação de Iluminação
    float lambert = max(0,dot(n,l));
    color.rgb = Kd0 * (lambert + 0.01);

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 

