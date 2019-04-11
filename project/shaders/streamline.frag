#version 430 core

out vec4 colour;

uniform bool fade;
uniform float maxDist;
uniform float minDist;
uniform float totalTime;

in vec4 C;
in vec3 L;
in vec3 V;
in vec3 T;

in float dist;
in float lt;

void main(void) {    	

	// Illuminated streamlines
	// "Interactive Visualization Of 3D-Vector Fields Using Illuminated Stream Lines" Zockler et al. 1996
	float lDt = dot(L, T);
	float vDt = dot(V, T);

	// N dot V
	float diffuse = sqrt(1.f - (lDt * lDt));
	diffuse *= diffuse;

	// R dot V
	float spec = max(diffuse * sqrt(1.f - (vDt * vDt)) - lDt * vDt, 0.f);

	// Phong reflection model
	float n = 32.f;
	vec3 ka = 0.2f * C.xyz;
	vec3 kd = 0.8f * diffuse * C.xyz;
	vec3 ks = 0.6f * pow(spec, n) * vec3(1.f, 1.f, 1.f);

	// Calculate alpha if fading is needed
	if (fade) {

		// Fall off function
		float norm = (dist - minDist) / (maxDist - minDist);
		norm = 1.f - 1.75f * norm;

		if (norm < 0.f) norm = 0.f;
		if (norm > 1.f) norm = 1.f;

		float q = 0.99996f;
		float repeat = 100000.f;

		float expon = totalTime - mod(lt, repeat);
		float alpha = 0.f;

		if (expon < 0.f) {
			expon += repeat;
		}
		alpha = pow(q, expon);
		colour = vec4(ka + kd + ks, C.w * norm * alpha);
	}
	else {
		colour = vec4(ka + kd + ks, C.w);
	}


}