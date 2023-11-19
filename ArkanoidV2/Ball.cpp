#include "Ball.h"
#include <random>

void Ball::initV() {
	int sign = 1;
	srand(static_cast<unsigned int>(clock()));
	float frac = (float(rand()) / (float(RAND_MAX)))/(float(rand()) / (float(RAND_MAX))+2.0f);
	frac *= 1000;
	frac -= floor(frac);
	frac /= 2;
	float a = float(rand()) / (float(RAND_MAX))-0.5;
	if (a < 0) {
		sign = -1;
	}
	v = glm::vec3(max*frac*sign, 0.0f, max*(1-frac));
}

void Ball::wallCollisions(Shield shield, int* i, int* j) {
	if (center.y > -1) {
		//boki
		if (center.x > 7) {
			v = glm::vec3(-abs(v[0]), v[1], v[2]);
			*i = -1;
			*j = -1;
		}	
		else if (center.x < -7) {
			v = glm::vec3(abs(v[0]), v[1], v[2]);
			*i = -1;
			*j = -1;
		}
		//góra
		if (center.y > 15.5) {
			*i = -1;
			*j = -1;
			v = glm::vec3(v[0], v[1], -v[2]);
		}
		//tarcza
		if (center.y < 0 && center.x>shield.X_left && center.x < shield.X_right) {
			float sign = -1.0f;
			*i = -1;
			*j = -1;
			float middle = (shield.X_left + shield.X_right)/2;
			float fraction = ((abs(center.x - middle)) / 3);
			if (center.x > middle) {
				sign = 1.0f;
			}
			v = glm::vec3(max * fraction * sign, 0.0f, max * (1 - fraction));
		}
	}

}