#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


inline glm::vec3 toVec3(const glm::vec2& _vec)
{
	return glm::vec3{ _vec.x, 0.0f, _vec.y };
}
