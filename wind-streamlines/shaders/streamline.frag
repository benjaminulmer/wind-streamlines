#version 430 core

out vec4 colour;

uniform float totalTime;
uniform float timeMultiplier;
uniform float timeRepeat;
uniform float alphaPerSecond;

uniform float specularToggle; // value of 0 turns off spec
uniform float diffuseToggle; // value of 1 turns off diff

in vec3 C;
in vec3 L;
in vec3 V;
in vec3 T;

in float t;

void main(void) {    	

	// Illuminated streamlines
	// "Interactive Visualization Of 3D-Vector Fields Using Illuminated Stream Lines" Zockler et al. 1996
	float lDt = dot(L, T);
	float vDt = dot(V, T);

	// L dot N
	float diffuse = max(sqrt(1.f - (lDt * lDt)), diffuseToggle);
	diffuse *= diffuse;

	// R dot V
	float spec = max(diffuse * sqrt(1.f - (vDt * vDt)) - lDt * vDt, 0.f);

	// Phong reflection model
	float n = 32.f;
	vec3 ka = 0.2f * C;
	vec3 kd = 0.8f * diffuse * C;
	vec3 ks = 0.6f * pow(spec, n) * vec3(1.f, 1.f, 1.f) * specularToggle;

	float expon = mod(totalTime - t, timeRepeat) / timeMultiplier;
	float alpha = pow(alphaPerSecond, expon);
	
	colour = vec4(ka + kd + ks, alpha);
}