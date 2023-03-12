#pragma once
#include "glm/vec3.hpp"

struct Vertex
{
	glm::vec3 Position = glm::vec3(0.f);
	float U = 0.5f;
	float V = 0.5f;
	float ColorR = 1;
	float ColorG = 1.f;
	float ColorB = 1.f;
	float ColorA = 1;
	glm::vec3 Normal = glm::vec3(0.f, 1.f, 0.f);
};