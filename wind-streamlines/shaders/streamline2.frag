#version 430 core

out vec4 colour;

uniform float totalTime;
uniform float timeMultiplier;
uniform float timeRepeat;
uniform float alphaPerSecond;

in float t;

void main(void) {    	

	float expon = mod(totalTime - t, timeRepeat) / timeMultiplier;
	float alpha = pow(alphaPerSecond, expon);
	
	colour = vec4(0.f, 0.f, 0.f, alpha);
}