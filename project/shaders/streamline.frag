#version 430 core

out vec4 colour;

uniform bool fade;
uniform float maxDist;
uniform float minDist;
uniform float totalTime;
uniform float timeMultiplier;
uniform float timeRepeat;
uniform float alphaPerSecond;
uniform float specularToggle;

in vec3 C;
in vec3 L;
in vec3 V;
in vec3 T;

in float d;
in float t;

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
	vec3 ka = 0.2f * C;
	vec3 kd = 0.8f * diffuse * C;
	vec3 ks = 0.6f * pow(spec, n) * vec3(1.f, 1.f, 1.f) * specularToggle;

	// Calculate alpha if fading is needed
	if (fade) {

		// Fall off function
		float norm = (d - minDist) / (maxDist - minDist);
		norm = 1.f - 1.75f * norm;

		if (norm < 0.f) norm = 0.f;
		if (norm > 1.f) norm = 1.f;

		float expon = mod(totalTime - mod(t, timeRepeat), timeRepeat) / timeMultiplier;
		float alpha = pow(alphaPerSecond, expon);
		
		colour = vec4(ka + kd + ks, norm * alpha);
	}
	else {
		colour = vec4(ka + kd + ks, 1.f);
	}


}