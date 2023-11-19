#pragma once
#include "Model.h"
#include "Shield.h"
class Ball : public Model
{
	using Model::Model;

public:
	glm::vec3 v;
	glm::vec2 center = glm::vec2(0.0f, 0.5f);
	float max = 0.15f;
	float r = 0.5;

	void initV();
	void wallCollisions(Shield shield,int* i, int* j);
};

