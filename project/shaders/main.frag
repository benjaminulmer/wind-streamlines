#version 430 core

out vec4 colour;

uniform bool fade;
uniform float maxDist;
uniform float minDist;

in vec4 C;

in float dist;

void main(void) {    	

	// Calculate alpha if fading is needed
	if (fade) {

		// Fall off function
		float norm = (dist - minDist) / (maxDist - minDist);
		norm = 1.f - 1.75f * norm;

		if (norm < 0.f) norm = 0.f;
		if (norm > 1.f) norm = 1.f;

		colour = vec4(C.xyz, C.w * norm);
	}
	else {
		colour = C;
	}
}