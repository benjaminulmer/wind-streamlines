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


	vec3 B = normalize(cross(T, V));
	vec3 N = normalize(cross(B, T));
	vec3 Lproj = L - (T * lDt);

	float theta = acos(dot(N, normalize(Lproj)));

	// L dot N
	float lDn = max(sqrt(1.f - (lDt * lDt)), diffuseToggle);
	float diffuse = lDn * (sin(theta) + (3.14159 - theta) * cos(theta)) / 4.f;

	// R dot V
	float spec = max(lDn * sqrt(1.f - (vDt * vDt)) - lDt * vDt, 0.f);

	// Phong reflection model
	float n = 32.f;
	vec3 ka = 0.2f * C;
	vec3 kd = 0.8f * diffuse * C;
	vec3 ks = 0.6f * pow(spec, n) * vec3(1.f, 1.f, 1.f) * specularToggle;

	float expon = mod(totalTime - t, timeRepeat) / timeMultiplier;
	float alpha = pow(alphaPerSecond, expon);
	
	colour = vec4(ka + kd + ks, alpha);
}