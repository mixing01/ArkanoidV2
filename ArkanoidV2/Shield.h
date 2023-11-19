#pragma once
#include "Model.h"
class Shield : public Model
{
	using Model::Model;

public:
	float X_left = -1.5f;
	float X_right = 1.5f;
};

