#version 430 core

out vec4 colour;

uniform bool fade;
uniform float maxDist;
uniform float minDist;

uniform bool hasTexture;
uniform sampler2D imTexture;

in vec3 C;
in vec3 L;
in vec3 V;
in vec2 UV;

void main(void) {    	

	if (hasTexture) {
		colour = texture(imTexture, UV);
	}
	// Calculate alpha if fading is needed
	else if (fade) {
		float dist = length(V);

		// Fall off function
		float norm = (dist - minDist) / (maxDist - minDist);
		norm = 1.f - 1.75f * norm;

		if (norm < 0.f) norm = 0.f;
		if (norm > 1.f) norm = 1.f;

		colour = vec4(C, norm);
	}
	else {
		colour = vec4(C, 1.0);
	}
}