#version 430 core

uniform mat4 modelView;
uniform mat4 projection;

uniform vec3 eyeHigh;
uniform vec3 eyeLow;

uniform float altScale;
uniform float radiusEarthM;

uniform bool scale;

layout (location = 0) in vec3 vertexHigh;
layout (location = 1) in vec3 vertexLow;
layout (location = 2) in vec3 colour;
layout (location = 3) in vec3 c2;
layout (location = 4) in vec3 tangent;
layout (location = 5) in float localTime;

out vec3 C;
out vec3 L;
out vec3 V;
out vec3 T;

out float t;

void main(void) {	

	float len = length(vertexHigh);
	vec3 vertexHighS = normalize(vertexHigh);

	float newLength = (len - radiusEarthM) * altScale + radiusEarthM;
	vertexHighS *= newLength;

	vec3 t1 = vertexLow - eyeLow;
	vec3 e = t1 - vertexLow;
	vec3 t2 = ((-eyeLow - e) + (vertexLow - (t1 - e))) + vertexHighS - eyeHigh;
	vec3 highDiff = t1 + t2;
	vec3 lowDiff = t2 - (highDiff - t1);

	vec3 vertex = highDiff + lowDiff;
	vec3 lightPos = eyeHigh;

	L = normalize(lightPos - vertexHigh);
	V = normalize(eyeHigh - vertexHigh);
	T = tangent;
	C = colour;
	if (!scale)
		C = c2;
	t = localTime;

	vec4 pCamera = modelView * vec4(vertex, 1.f);

    gl_Position = projection * pCamera; 
}
