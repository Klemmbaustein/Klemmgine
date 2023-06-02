#pragma once
#include "glm/vec3.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Vertex
{
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec2 TexCoord = glm::vec2(0.0f);
	glm::vec4 Color = glm::vec4(1.0);
	glm::vec3 Normal = glm::vec3(0.f, 1.f, 0.f);
};