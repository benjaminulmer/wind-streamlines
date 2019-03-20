#version 430 core

uniform mat4 modelView;
uniform mat4 projection;

uniform vec3 eyeHigh;
uniform vec3 eyeLow;

layout (location = 0) in vec3 vertexHigh;
layout (location = 1) in vec3 vertexLow;
layout (location = 2) in vec3 colour;
layout (location = 3) in vec2 uv;

out vec3 C;
out vec3 L;
out vec3 V;
out vec2 UV;

void main(void) {	

	vec3 t1 = vertexLow - eyeLow;
	vec3 e = t1 - vertexLow;
	vec3 t2 = ((-eyeLow - e) + (vertexLow - (t1 - e))) + vertexHigh - eyeHigh;
	vec3 highDiff = t1 + t2;
	vec3 lowDiff = t2 - (highDiff - t1);

	vec3 vertex = highDiff + lowDiff;

	C = colour;
	UV = uv;

	vec3 lightPos = vec3(0.0, 10.0, 0.0);

	// Put light in camera space
	vec4 lightCameraSpace = modelView * vec4(lightPos, 1.0);
	

	// Transform model and put in camera space
    vec4 pCameraSpace = modelView * vec4(vertex, 1.0); 
	vec3 P = pCameraSpace.xyz;
	
	// Calculate L and V vectors
	L = normalize(lightCameraSpace.xyz - P);
	V = -P;

    gl_Position = projection * pCameraSpace;   
}
