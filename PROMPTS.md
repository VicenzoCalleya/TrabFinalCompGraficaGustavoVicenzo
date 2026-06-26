Prompts Gustavo:

PROMPT 1:

(prompt genérico de implementação de modelo, que usei algumas vezes)

Use o arquivo x.obj e sua respectiva textura, para substituir o modelo y do projeto (modelo que está como placeholder). Leia as informaçẽos do .obj.

**Citações aos obj e texturas dos objetos na main e no shader fragment, foram geradas assim**

PROMPT 2:

As pupilas dos pokémons está branca, tem algo de errado na textura, como posso arrumar isso?

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
    else if ( object_id == BULBASAUR_EYES )
    {
        vec3 normalized_pos = (position_model.xyz - bbox_min.xyz) / (bbox_max.xyz - bbox_min.xyz);
        vec2 eye_center = vec2(0.5, 0.5);
        float dist_from_center = length(normalized_pos.xy - eye_center);
        if (dist_from_center < 0.08) {
            Kd0 = vec3(0.02, 0.02, 0.02);
        } else if (dist_from_center < 0.3) {
            Kd0 = mix(vec3(0.95, 0.95, 0.95), vec3(0.1, 0.1, 0.1), (dist_from_center - 0.08) / 0.22);
        } else {
            Kd0 = vec3(0.85, 0.85, 0.85);
        }
    }

PROMPT 3:

Como implemento iluminação para os modelos? Para que gerem sombra no solo. 

      uniform bool is_shadow;

      void main()
        {
            // Se for sombra, pinta de escuro e encerra a execução do shader para este fragmento
            if (is_shadow) {
                color = vec4(0.1, 0.1, 0.1, 1.0); // Cor da sombra
                return;
        }

        GLint g_is_shadow_uniform = glGetUniformLocation(g_GpuProgramID, "is_shadow");

        glm::vec4 light_dir = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f); 
        
        // Colocamos a sombra levemente acima do chão (-1.1f) para evitar Z-fighting (cintilação)
        float ground_y = -1.095f; 

        glm::mat4 S = Matrix_Identity();
        if (light_dir.y != 0.0f) {
            S[1][0] = -light_dir.x / light_dir.y;
            S[1][1] = 0.0f;
            S[1][2] = -light_dir.z / light_dir.y;
            S[3][0] = ground_y * (light_dir.x / light_dir.y);
            S[3][1] = ground_y;
            S[3][2] = ground_y * (light_dir.z / light_dir.y);
        }

        // Prepara OpenGL para as sombras
        glUniform1i(g_is_shadow_uniform, 1); // Avisa o shader que é sombra
        glDisable(GL_CULL_FACE); // Desativa Culling, pois achatar o modelo inverte a ordem de alguns triângulos

        // Desenha a sombra do treinador
        glm::mat4 player_model = Matrix_Translate(g_PlayerX, ground_y, g_PlayerZ)
                               * Matrix_Rotate_Y(g_AngleY)
                               * Matrix_Scale(0.3f, 0.3f, 0.3f);
        glm::mat4 player_shadow = S * player_model;
        
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(player_shadow));
        glUniform1i(g_object_id_uniform, TRAINER);
        DrawVirtualObjectByPattern("BASE", TRAINER);

        // Desenha as sombras do resto do mundo
        for (const auto& obj : g_GameWorld)
        {
            // Não queremos projetar sombra do próprio chão nem do capim pequeno
            if (obj.object_id == PLANE || obj.object_id == GRASS) continue;

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

        // Restaura o estado original do OpenGL
        glEnable(GL_CULL_FACE);
        glUniform1i(g_is_shadow_uniform, 0);


PROMPT 4:

Tenho esse sistema de sombras. E quero implementar nele uma nova funcionalidade.


Quando uma pokebola estiver no chão depois de capturar um pokémon (depois do jogador apertar E, e então segurar o botão direito do mouse, soltar e a pokébola atingir um pokémon), quero que até que ela suma, as sombras em volta dela sumam em uma área circular, como se ela estivesse iluminando essa área, e que os objetos dentro dessa área circular imitam sombras, como se temporariamente a pokébola fosse uma outra fonte de luz além da que vem de cima, mas apenas para os objetos dentro da área, e que a sombra seja achatada de forma condizente ao ponto de luz de onde a pokebola está. 


        uniform bool is_shadow;
        
        // --- NOVAS UNIFORMS PARA LUZ DA POKÉBOLA ---
        uniform vec3 pokeball_pos;
        uniform float pokeball_radius;
        uniform int shadow_type; // 0 = Sombra do sol, 1 = Sombra da pokébola
        // -------------------------------------------

        OpenGL Shading Language
            if (is_shadow) {
                if (pokeball_radius > 0.0) {
                    // position_world divido por W é boa prática matemática
                    vec2 p_xz = position_world.xz / position_world.w;
                    float dist = distance(p_xz, pokeball_pos.xz);
            
            // Máscara Circular: apaga a sombra do sol dentro do raio, 
            // e apaga a sombra da pokébola fora do raio.
            if (shadow_type == 0 && dist < pokeball_radius) {
                discard; 
            } else if (shadow_type == 1 && dist > pokeball_radius) {
                discard; 
            }
        }
        
        // Cor da sombra (cinza escuro)
        color = vec4(0.1, 0.1, 0.1, 1.0);
        return; 
    }

    // Equação de Iluminação
    float lambert = max(0.0, dot(n,l));
    vec3 final_color = Kd0 * (lambert + 0.01);

    // Se houver uma pokébola ativa, ilumina a área ao redor dela
    if (pokeball_radius > 0.0) {
        vec3 p_world = position_world.xyz / position_world.w;
        float dist = distance(p_world.xz, pokeball_pos.xz);
        
        // Vetor de luz pontual vindo da pokébola
        vec3 light_dir_pb = normalize(vec3(pokeball_pos.x, pokeball_pos.y + 1.0, pokeball_pos.z) - p_world);
        float lambert_pb = max(0.0, dot(n.xyz, light_dir_pb));
        
        // Atenuação radial da luz (vai perdendo a força até a borda)
        float atenuacao = smoothstep(pokeball_radius, 0.0, dist);
        
        // Adicionando um brilho à cor base
        final_color += Kd0 * lambert_pb * atenuacao * 0.8;
    }

    color.rgb = final_color;

    // ========================================================
        // PREPARAÇÃO PARA O SISTEMA DE SOMBRAS
        // ========================================================
        // 1. Encontra a Pokébola (se existir e estiver no chão)
        bool has_active_pokeball = false;
        glm::vec3 active_pokeball_pos(0.0f);
        for (const auto& obj : g_GameWorld) {
            if (obj.is_pokebola && obj.hit_ground) {
                has_active_pokeball = true;
                active_pokeball_pos = obj.position;
                break; // Usa a primeira que encontrar
            }
        }
        float pb_radius = 6.0f; // Tamanho da área afetada

        // Envia dados para o Shader
        glUniform3f(glGetUniformLocation(g_GpuProgramID, "pokeball_pos"), active_pokeball_pos.x, active_pokeball_pos.y,     active_pokeball_pos.z);
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
            if (obj.object_id == PLANE || obj.object_id == GRASS) continue;

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
                if (obj.object_id == PLANE || obj.object_id == GRASS || obj.is_pokebola) continue;

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

Prompts Vicenzo:

PROMPT 1: Faça todos os objetos que tenham a flag "is_solid" como verdadeiro terem uma BB, em que o jogador não pode atravessar ela, impedindo ele de continuar uma vez que ele colida com um objeto sólido (pokemons, árvores, ect)

    void InitializeMap() {
        // Certifique-se de limpar o vetor antes, caso a função seja chamada mais de uma vez
        g_GameWorld.clear();

    // 1. O Chão
    GameObject chao;
    chao.model_name = "the_plane";
    chao.object_id  = PLANE;
    chao.position   = glm::vec3(0.0f, -1.1f, 0.0f);
    chao.scale      = glm::vec3(100.0f, 1.0f, 100.0f); // Se a escala for 0, ele some!
    chao.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    chao.is_solid   = false;
    g_GameWorld.push_back(chao);

    // 2. O Coelho
    GameObject coelho;
    coelho.model_name = "the_bunny";
    coelho.object_id  = BUNNY;
    coelho.position   = glm::vec3(5.0f, 0.0f, 0.0f);
    coelho.scale      = glm::vec3(1.0f, 1.0f, 1.0f);   // Garante escala 1
    coelho.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    coelho.is_solid   = true;
    g_GameWorld.push_back(coelho);
    }
    
    // ... Dentro da função main() ...
    
    // 1. Primeiro carrega os modelos do disco para a memória
    BuildTrianglesAndAddToVirtualScene(...); 
    
    // 2. DEPOIS você inicializa a lista de objetos do jogo
    InitializeMap(); 
    
    // 3. Só então entra no loop principal
    while (!glfwWindowShouldClose(window)) {
        // ...
    }

    for (const auto& obj : g_GameWorld)
    {
        model = Matrix_Translate(obj.position.x, obj.position.y, obj.position.z)
              * Matrix_Rotate_Y(obj.rotation.y)
              * Matrix_Rotate_X(obj.rotation.x)
              * Matrix_Rotate_Z(obj.rotation.z)
              * Matrix_Scale(obj.scale.x, obj.scale.y, obj.scale.z); 
              // Se suspeitar da escala do chão, teste trocar temporariamente por:
              // * Matrix_Scale(1.0f, 1.0f, 1.0f);

    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, obj.object_id);
    DrawVirtualObject(obj.model_name.c_str());
    }
    
    g_SceneColliders.clear();

        // Varremos todos os objetos do jogo para atualizar suas hitboxes
        for (const auto& obj : g_GameObjects) 
        {
            if (obj.is_solid) 
            {
                Collider c;
                // A MAGICA: A hitbox acompanha a posição atual do objeto automaticamente!
                // Se o objeto se moveu no frame, a colisão se move junto.
                c.min = g_VirtualScene[obj.model_name].bbox_min + obj.position;
                c.max = g_VirtualScene[obj.model_name].bbox_max + obj.position;
                
                g_SceneColliders.push_back(c);
            }
        }


PROMPT 2: Faça os pokémons se moverem por meio de uma curva de Bézier cúbica codificada, onde que é possível colocar a trajetória informando os pontos de controle e pontos de inicio e fim. Os pokemons vão e voltam por essa trajetória, e param se colidirem com o jogador, continuando uma vez que ele sair do caminho.
    
    struct GameObject {
        std::string model_name; 
        int object_id;          
        glm::vec3 position;     
        glm::vec3 scale;        
        glm::vec3 rotation;     
        bool is_solid;          

    // --- NOVOS CAMPOS PARA A CURVA DE BÉZIER ---
    bool is_moving_bezier; // Se true, o objeto se move por curvas
    glm::vec3 p0;          // Ponto inicial
    glm::vec3 p1;          // Ponto de controle (curvatura)
    glm::vec3 p2;          // Ponto final
    float t_bezier;        // Tempo paramétrico atual (vai de 0.0f a 1.0f)
    float speed_bezier;    // Velocidade de deslocamento do t
    };

    void InitializeMap() {
        g_GameWorld.clear();

    // 1. O Chão (continua igual)
    GameObject chao;
    chao.model_name = "the_plane";
    chao.object_id  = PLANE;
    chao.position   = glm::vec3(0.0f, -1.1f, 0.0f);
    chao.scale      = glm::vec3(100.0f, 1.0f, 100.0f); 
    chao.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    chao.is_solid   = false;
    chao.is_moving_bezier = false; // O chão não se move
    g_GameWorld.push_back(chao);
    
    // 2. O Coelho (Pokémon Selvagem)
    GameObject coelho;
    coelho.model_name = "the_bunny";
    coelho.object_id  = BUNNY;
    coelho.position   = glm::vec3(5.0f, 0.0f, 0.0f); // Posição inicial inicial
    coelho.scale      = glm::vec3(1.0f, 1.0f, 1.0f);   
    coelho.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
    chao.is_solid     = true;

    // Configuração do movimento de Bézier:
    coelho.is_moving_bezier = true;
    coelho.p0           = glm::vec3(5.0f, 0.0f, 0.0f);   // Início
    coelho.p1           = glm::vec3(10.0f, 0.0f, 5.0f);  // Ponto de curva
    coelho.p2           = glm::vec3(5.0f, 0.0f, 10.0f);  // Segundo ponto de curva
    coelho.p3           = glm::vect3(10.0f, 0.0f, 15.0f);// Fim
    coelho.t_bezier     = 0.0f;                          // Começa no início da curva
    coelho.speed_bezier = 0.2f;                          // Velocidade da caminhada
    
    g_GameWorld.push_back(coelho);
    }
    
    // Dentro do loop
    for (auto& obj : g_GameWorld){
    if (obj.is_moving_bezier) 
    {
        obj.bezier_t += delta_time * 0.2f;
        
       if (!obj.is_returning) {
       	obj.t_bezier += obj.speed_bezier * delta_time;
            if (obj.t_bezier >= 1.0f) {
            obj.t_bezier = 1.0f;
            	obj.is_returning = true; // Chegou no fim, começa a voltar
            }
       } else {
            obj.t_bezier -= obj.speed_bezier * delta_time;
            if (obj.t_bezier <= 0.0f) {
            	obj.t_bezier = 0.0f;
                    obj.is_returning = false; // Chegou no início, vai para a frente
            }
       }

    float t = obj.bezier_t;
    float inv_t = 1.0f - t;

    // 1. CÁLCULO DA POSIÇÃO
    float b0 = inv_t * inv_t * inv_t;
    float b1 = 3.0f * inv_t * inv_t * t;
    float b2 = 3.0f * inv_t * t * t;
    float b3 = t * t * t;

    obj.position = (outro_obj.p0 * b0) + (outro_obj.p1 * b1) + (outro_obj.p2 * b2) + (outro_obj.p3 * b3);

    glm::vec3 bunny_min = g_VirtualScene[obj.model_name].bbox_min + obj.position;
    glm::vec3 bunny_max = g_VirtualScene[obj.model_name].bbox_max + obj.position;

    // 5. TESTE DE INTERCEPTAÇÃO: O coelho colidiu com a posição ATUAL do jogador?
    if (CheckCollision_AABB(player_bbox_min, player_bbox_max, bunny_min, bunny_max)) 
    {
    	// Se colidir, o Pokémon "trava" no lugar: desfaz a posição e o tempo
        obj.position = old_bunny_pos;
        obj.t_bezier = old_t;
    }

    else{

    	// ========================================================
    	// 2. CÁLCULO DE ROTAÇÃO (Vetor Tangente)
    	// ========================================================
    	// Derivada dos polinômios de Bernstein cúbicos:
    	float d0 = 3.0f * inv_t * inv_t;
    	float d1 = 6.0f * inv_t * t;
    	float d2 = 3.0f * t * t;

    	// Vetor de direção/velocidade instantânea no frame
   	 glm::vec3 tangente = d0 * (outro_obj.p1 - outro_obj.p0) +
                              d1 * (outro_obj.p2 - outro_obj.p1) +
                              d2 * (outro_obj.p3 - outro_obj.p2);

    	// Calculamos o ângulo no plano XZ (ignoring Y para rotação horizontal)
    	// O atan2 retorna o arco tangente em radianos
    	float angulo_radianos = atan2(tangente.x, tangente.z);

    	// Se o seu motor/shader usa graus para o GameObject:
    	// Mude apenas o eixo Y para rotacionar o Pokémon na direção do movimento
   	 obj.rotation.y = angulo_radianos * (180.0f / M_PI); 
    
    // NOTA: Se o modelo do Pokémon importado de costas ou de lado por padrão, 
    // basta somar ou subtrair a diferença em graus aqui, por exemplo:
    // obj.rotation.y = (angulo_radianos * (180.0f / M_PI)) + 180.0f;
    }
    }



PROMPT 3: Codifique o lançamento de pokebolas, que funciona assim:


Primeiramente, há um segundo modo de câmera: mira, que basicamente a rotação do jogador não depende mais de onde ele se move, mas sim de onde a câmera está apontando. Para ficar nesse modo, o jogador precisa pressionar "E". Para jogar uma pokebola, o jogador pressiona o botão direito do mouse, ele uma pokebola é lançada dependendo de

Onde a câmera está apontando (a direção)

Por quanto tempo o botão direito foi pressionado (força)

A animação da pokebola é calculada a partir de tempo, e ela pode colidir com qualquer objeto sólido. Se ela colidir com qualquer coisa que não for um pokemon, ela quica e continua sua trajetória. Se for um pokemon, porém, o pokémon desaparece, a pokebola fica no chão, faz uma animação (não e concentre nela por enquanto) e desaparece. De qualquer maneira, uma vez que ela encosta no chão, ela desaparece após 5 segundos

    struct GameObject {
        std::string model_name; 
        int object_id;          
        glm::vec3 position;     
        glm::vec3 scale;        
        glm::vec3 rotation;     
        bool is_solid;          

    // --- BÉZIER (POKÉMONS) ---
    bool is_moving_bezier; 
    glm::vec3 p0, p1, p2;
    float t_bezier;        
    float speed_bezier;    
    bool is_returning;     

    // --- NOVO: PROPRIEDADES DA POKÉBOLA ---
    bool is_pokebola;       // Identifica se o objeto é uma pokébola ativa
    glm::vec3 launch_dir;   // Direção do arremesso
    float force;            // Força acumulada
    float live_time;        // Contador de tempo (para o limite de 5s)
    bool hit_ground;        // Se já colidiu e está estática no chão
    };
    
    bool g_AimMode = false;
    float g_RightClickDuration = 0.0f; // Guarda quanto tempo segurou o clique
    
    // ========================================================
            // CONTROLE DO MODO DE MIRA (TECLA E)
            // ========================================================
            // Se o jogador pressionar E (ajuste com base na sua flag de leitura de tecla)
            if (g_E_Pressed_Modulo_Toggle) { 
                g_AimMode = !g_AimMode; 
            }

        // ... Seu código de cálculo de move_direction (W, A, S, D) continua igual ...

        if (norm(move_direction) > 0.0f)
        {
            move_direction = move_direction / norm(move_direction);
            g_PlayerX += move_direction.x * player_speed;
            g_PlayerZ += move_direction.z * player_speed;

            // Se NÃO está mirando, o corpo gira para a direção do movimento (3ª pessoa livre)
            if (!g_AimMode) {
                g_AngleY = atan2f(move_direction.x, move_direction.z);
            }
        }

        // Se ESTÁ mirando, o corpo ignora o movimento e trava na mira da câmera!
        if (g_AimMode) {
            // camera_view_vector.x e z determinam a rotação do corpo do jogador
            g_AngleY = atan2f(camera_view_vector.x, camera_view_vector.z);
        }

    // ========================================================
            // ACUMULO DE FORÇA E LANÇAMENTO DA POKÉBOLA
            // ========================================================
            if (g_AimMode && g_RightMouseButtonPressed) 
            {
                // Acumula força enquanto o botão estiver pressionado (limite máximo de 3.0 segundos)
                g_RightClickDuration += delta_time;
                if (g_RightClickDuration > 3.0f) g_RightClickDuration = 3.0f;
            }
            // No frame em que o jogador SOLTA o botão direito
            else if (g_AimMode && g_RightClickDuration > 0.0f) 
            {
                // Instancia uma nova Pokébola no mundo!
                GameObject pokebola;
                pokebola.model_name = "the_sphere"; // Placeholder esférico do lab
                pokebola.object_id  = POKEBOLA;     // Garanta que esse ID exista no seu enum
                
            // Nasce um pouco à frente do peito do jogador
            pokebola.position   = glm::vec3(g_PlayerX, g_PlayerY + 0.5f, g_PlayerZ) + (glm::vec3(camera_view_vector.x, 0.0f, camera_view_vector.z) * 0.8f);
            pokebola.scale      = glm::vec3(0.2f, 0.2f, 0.2f); // Pokébolas são pequenas!
            pokebola.rotation   = glm::vec3(0.0f, 0.0f, 0.0f);
            pokebola.is_solid   = true;
            
            // Configurações específicas do arremesso
            pokebola.is_pokebola       = true;
            pokebola.is_moving_bezier  = false; // Ela usa outra física, não o Bézier dos Pokémons
            pokebola.launch_dir        = glm::vec3(camera_view_vector.x, camera_view_vector.y, camera_view_vector.z);
            pokebola.force             = g_RightClickDuration; // Força baseada no tempo segurado
            pokebola.live_time         = 0.0f;
            pokebola.hit_ground        = false;

            // Injeta a pokébola na nossa lista global dinâmica!
            g_GameWorld.push_back(pokebola);

            // Reseta o contador para o próximo arremesso
            g_RightClickDuration = 0.0f;
        }

	// [DENTRO DO IF (!OBJ.HIT_GROUND) DA POKÉBOLA]
                    
                    // 1. Calcula a hitbox atual da Pokébola voando
                    glm::vec3 poke_min = g_VirtualScene[obj.model_name].bbox_min + obj.position;
                    glm::vec3 poke_max = g_VirtualScene[obj.model_name].bbox_max + obj.position;

                    // 2. Passa por todos os objetos do mundo para ver se colidiu
                    for (auto& outro_obj : g_GameWorld)
                    {
                        // Filtro: Não colidir com ela mesma, nem com o chão plano
                        if (outro_obj.object_id == PLANE) continue;

                        // NOVIDADE: Se o objeto do cenário NÃO for sólido (ex: grama), a pokébola ignora!
                        // Deixamos os Pokémons passarem por aqui mesmo sem is_solid para podermos capturá-los
                        if (!outro_obj.is_solid && outro_obj.object_id != CHARMANDER && outro_obj.object_id != SQUIRTLE) continue;

                        // Calcula a caixa do objeto alvo
                        glm::vec3 target_min = g_VirtualScene[outro_obj.model_name].bbox_min + outro_obj.position;
                        glm::vec3 target_max = g_VirtualScene[outro_obj.model_name].bbox_max + outro_obj.position;

                        if (CheckCollision_AABB(poke_min, poke_max, target_min, target_max))
                        {
                            // SE COLIDIR COM UM POKÉMON (Charmander ou Squirtle)
                            if (outro_obj.object_id == CHARMANDER || outro_obj.object_id == SQUIRTLE)
                            {
                                printf("Capturou o Pokemon!\n");
                                outro_obj.is_moving_bezier = false;
                                outro_obj.is_solid = false;
                                outro_obj.position.y = -10.0f; // Some com o pokémon
                                
                                // A pokébola para no chão onde capturou
                                obj.hit_ground = true;
                                obj.position.y = -1.1f;
                                obj.rotation.x = 0.0f;
                                break; 
                            }
                            // SE COLIDIR COM OBJETO SÓLIDO DE CENÁRIO (Árvores, Paredes, etc.) -> EFEITO DE QUIQUE
                            else 
                            {
                                printf("Bateu e quicou!\n");

                                // Calcula a sobreposição (overlap) em cada eixo para saber por onde bateu
                                float overlapX = std::min(poke_max.x, target_max.x) - std::max(poke_min.x, target_min.x);
                                float overlapZ = std::min(poke_max.z, target_max.z) - std::max(poke_min.z, target_min.z);

                                // Se a menor sobreposição for no eixo X, ela bateu nas laterais do objeto (Leste/Oeste)
                                if (overlapX < overlapZ) 
                                {
                                    obj.launch_dir.x = -obj.launch_dir.x; // Inverte direção horizontal X
                                    // Afasta ligeiramente para evitar que fique presa dentro do colisor no próximo frame
                                    obj.position.x += (obj.launch_dir.x > 0 ? 1.0f : -1.0f) * overlapX;
                                } 
                                // Caso contrário, bateu nas faces de frente ou trás (Norte/Sul)
                                else 
                                {
                                    obj.launch_dir.z = -obj.launch_dir.z; // Inverte direção horizontal Z
                                    // Afasta ligeiramente no eixo Z
                                    obj.position.z += (obj.launch_dir.z > 0 ? 1.0f : -1.0f) * overlapZ;
                                }

                                // Perda de energia (amortecimento): reduz a força do quique em 30% para ficar realista
                                obj.force *= 0.7f;
                                
                                // Se a força ficar muito baixa após quicar, faz ela parar no chão de uma vez
                                if (obj.force < 0.2f) {
                                    obj.hit_ground = true;
                                    obj.position.y = -1.1f;
                                    obj.rotation.x = 0.0f;
                                }
                                
                                break; // Sai do laço de colisão deste frame
                            }
                        }
                    }


PROMPT 4: Codifique uma mecânica que se trata de que quando um pokemon é capturado, existe % em que ele volta para o mapa depois de 5 segundos, pois a captura "falhou", essa % depende de 3 fatores:


    
    - Se o jogador está agachado: É necessário adicionar uma mecânica de ficar agachado, como não temos shaders pra isso, apenas afundar o jogador para metade do tamanho dele, a tecla Q é o que faz ele agachar e levantar
    
    
    
    - Se o jogador acertou por trás: Se o jogar e acertar a pokebola nas costas do modelo do pokemon, a chance de capturar aumenta
    
    
    
    - Se o jogador está posicionado numa grama alta: Há várias gramas altas que o jogador pode ficar, e estar nela quando foi jogado a pokebola aumenta chances de captura
    
    
    
    A chance base é 50%, e cada fator aumenta nessa %:
    
    - Agachar: 15%
    
    - Costas: 20%
    
    - Grama alta: 15%
    
    
    bool g_IsCrouching = false;
    bool g_IsInHighGrass = false; // Atualizada frame a frame no teste de colisão

    // --- NA STRUCT GAMEOBJECT ---
    struct GameObject {
        // ... variáveis anteriores ...
    
    // Novas variáveis para a Pokébola
    bool is_pokebola;
    bool bonus_crouch;      // Se o jogador estava agachado no disparo
    bool bonus_grass;       // Se o jogador estava na grama no disparo
    
    // Novas variáveis para o Pokémon
    bool is_captured_loop;   // Se está na animação/teste de captura
    float capture_timer;     // Conta os 5 segundos dentro da bola
    size_t trapped_poke_idx; // A pokébola guarda qual índice do vetor ela capturou
    };

    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
    // ... suas outras teclas ...

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        g_IsCrouching = !g_IsCrouching;
        
        if (g_IsCrouching) {
            g_PlayerY = -1.4f; // Afunda o Y do jogador (ajuste conforme a escala do seu modelo)
        } else {
            g_PlayerY = -1.1f; // Volta à altura normal
        }
    }
    }

    else if (g_AimMode && g_RightClickDuration > 0.0f) 
        {
            GameObject pokebola;
            // ... (suas configurações anteriores de posição, escala, launch_dir) ...
            
            pokebola.is_pokebola = true;
            pokebola.hit_ground  = false;
            
            // SALVA OS BÔNUS DE STEALTH DO DISPARO
            pokebola.bonus_crouch = g_IsCrouching;
            pokebola.bonus_grass  = g_IsInHighGrass;
            
            pokebola.force       = g_RightClickDuration; 
            pokebola.live_time   = 0.0f;                 
            g_GameWorld.push_back(pokebola);

            g_RightClickDuration = 0.0f;
        }

    // Reseta a flag a cada frame antes de testar
        g_IsInHighGrass = false; 

        for (const auto& collider : g_SceneColliders) 
        {
            // Se o colisor for uma grama alta (identificada pelo ID do objeto que você definir)
            if (collider.object_id == HIGH_GRASS) 
            {
                glm::vec3 grass_center = (collider.min + collider.max) * 0.5f;
                float dx = g_PlayerX - grass_center.x;
                float dz = g_PlayerZ - grass_center.z;
                float dist = sqrt(dx*dx + dz*dz);
                
                if (dist < 1.8f) { // Raio da área da grama
                    g_IsInHighGrass = true;
                }
                continue; // Não bloqueia o jogador fisicamente
            }

            // Bloqueio físico normal de árvores e pokémons...
            if (CheckCollision_AABB(player_bbox_min, player_bbox_max, collider.min, collider.max)) {
                g_PlayerX = oldX;
                g_PlayerZ = oldZ;
                break; 
            }
        }

    // [DENTRO DO LOOP DE ATUALIZAÇÃO DA POKÉBOLA, SE COLIDIR COM POKÉMON]
                    if (collider.object_id == CHARMANDER || collider.object_id == SQUIRTLE)
                    {
                        printf("-> Pokébola atingiu o Pokémon! Iniciando teste de captura...\n");
                        
                        // Pegamos referências do Pokémon atingido
                        auto& pokemon = g_GameWorld[collider.world_index];
                        
                        // --- CÁLCULO DE ACERTO POR TRÁS (DOT PRODUCT) ---
                        // 1. Vetor de visão do Pokémon (baseado na rotação atual dele na curva)
                        float angle_rad = pokemon.rotation.y * (M_PI / 180.0f);
                        glm::vec3 poke_view = glm::vec3(sin(angle_rad), 0.0f, cos(angle_rad));
                        if (norm(poke_view) > 0.0f) poke_view = poke_view / norm(poke_view);

                        // 2. Vetor de direção da pokébola (apenas XZ)
                        glm::vec3 ball_dir = glm::vec3(obj.launch_dir.x, 0.0f, obj.launch_dir.z);
                        if (norm(ball_dir) > 0.0f) ball_dir = ball_dir / norm(ball_dir);

                        // Se o produto escalar for positivo, a bola viajava na mesma direção geral que o Pokémon olhava
                        // Significa que ela veio por trás e atingiu as costas!
                        float dot = dotproduct(poke_view, ball_dir);
                        bool bonus_backstab = (dot > 0.2f); 

                        // --- CÁLCULO DA CHANCE DE CAPTURA ---
                        float capture_chance = 0.50f; // 50% Base
                        if (obj.bonus_crouch)  capture_chance += 0.15f;
                        if (obj.bonus_grass)   capture_chance += 0.15f;
                        if (bonus_backstab)    capture_chance += 0.20f;

                        // Roda o dado (gerador de 0.0 a 1.0)
                        float random_roll = (float)rand() / (float)RAND_MAX;
                        bool capture_success = (random_roll <= capture_chance);

                        printf("Chance Total: %.2f%% | Dado: %.2f%% | Resultado: %s\n", 
                                capture_chance * 100.0f, random_roll * 100.0f, capture_success ? "SUCESSO" : "FALHA");

                        if (capture_success) 
                        {
                            // Sucesso definitivo: Deleta/Some com o Pokémon para sempre
                            pokemon.is_moving_bezier = false;
                            pokemon.is_solid = false;
                            pokemon.position.y = -10.0f; 
                        } 
                        else 
                        {
                            // Falha temporária: Ativa o modo de espera de 5 segundos
                            pokemon.is_moving_bezier = false;
                            pokemon.is_solid = false;
                            pokemon.position.y = -10.0f; // Esconde enquanto balança
                            
                            pokemon.is_captured_loop = true;
                            pokemon.capture_timer = 0.0f;
                        }

                        // Em ambos os casos, a pokébola pousa e encerra sua trajetória
                        obj.hit_ground = true;
                        obj.position.y = -1.1f;
                        obj.rotation.x = 0.0f;
                        break;
                    }

    // Loop de verificação de falha de captura (Coloque junto das atualizações no main)
        for (auto& pokemon : g_GameWorld)
        {
            if ((pokemon.object_id == CHARMANDER || pokemon.object_id == SQUIRTLE) && pokemon.is_captured_loop)
            {
                pokemon.capture_timer += delta_time;

                if (pokemon.capture_timer >= 5.0f)
                {
                    printf("-> O Pokémon escapou da Pokébola e voltou ao mapa!\n");
                    
                    // Restaura o Pokémon exatamente onde ele deveria estar na curva de Bézier
                    pokemon.is_captured_loop = false;
                    pokemon.is_moving_bezier = true;
                    pokemon.is_solid = true;
                    // O Y volta ao chão e a física do Bézier reposiciona o X e Z no próximo frame
                    pokemon.position.y = -1.1f; 
                }
            }
        }
