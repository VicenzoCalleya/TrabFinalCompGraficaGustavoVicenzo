#include "collisions.h"

bool CheckCollision_AABB(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
{
    // Verifica se há sobreposição no eixo X
    bool collisionX = maxA.x >= minB.x && minA.x <= maxB.x;
    
    // Verifica se há sobreposição no eixo Y
    bool collisionY = maxA.y >= minB.y && minA.y <= maxB.y;
    
    // Verifica se há sobreposição no eixo Z
    bool collisionZ = maxA.z >= minB.z && minA.z <= maxB.z;
    
    // Há colisão apenas se houver sobreposição nos 3 eixos simultaneamente
    return collisionX && collisionY && collisionZ;
}
