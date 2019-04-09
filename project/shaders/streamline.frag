#version 430 core

out vec4 colour;

uniform bool fade;
uniform float maxDist;
uniform float minDist;


in vec3 C;
in vec3 L;
in vec3 V;
in vec3 T;

in float dist;

void main(void) {    	

	float lDt = dot(L, T);
	float vDt = dot(V, T);

	float diffuse = sqrt(1.f - (lDt * lDt));
	diffuse *= diffuse;

	float spec = max(diffuse * sqrt(1.f - (vDt * vDt)) - lDt * vDt, 0.f);

	float n = 32.f;

	vec3 ka = 0.2f * C;
	vec3 kd = 0.8f * diffuse * C;
	vec3 ks = 0.6f * pow(spec, n) * vec3(1.f, 1.f, 1.f);

	// Calculate alpha if fading is needed
	if (fade) {

		// Fall off function
		float norm = (dist - minDist) / (maxDist - minDist);
		norm = 1.f - 1.75f * norm;

		if (norm < 0.f) norm = 0.f;
		if (norm > 1.f) norm = 1.f;

		colour = vec4(ka + kd + ks, norm);
	}
	else {
		colour = vec4(ka + kd + ks, 1.f);
	}


}