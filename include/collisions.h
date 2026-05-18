#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include <glm/vec3.hpp>

// Retorna true se houver colisão entre duas caixas AABB
bool CheckCollision_AABB(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB);

#endif // _COLLISIONS_H
