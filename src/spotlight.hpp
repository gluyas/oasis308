#pragma once

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"

class Spotlight {

private:


public:
	Spotlight();
	void renderSpotlight(cgra::vec3 trans, cgra::vec3 dir, float angle, float scale, cgra::vec3 rot);
};
