Prompts Gustavo:

PROMPT 1:

(prompt genérico de implementação de modelo, que usei algumas vezes)

Use o arquivo x.obj e sua respectiva textura, para substituir o modelo y do projeto (modelo que está como placeholder). Leia as informaçẽos do .obj.

# Citações aos obj e texturas dos objetos na main e no shader fragment, foram geradas assim

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
