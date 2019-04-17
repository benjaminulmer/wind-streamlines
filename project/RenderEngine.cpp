#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL
#include "RenderEngine.h"

#include "Conversions.h"
#include "Renderable.h"
#include "ShaderTools.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>


RenderEngine::RenderEngine(SDL_Window* window) :
	window(window),
	fovYRad(60.f * ((float)M_PI / 180.f)),
	near(1.f),
	far(1000.f),
	totalTime(0.f),
	timeMultiplier(30000.f),
	timeRepeat(100000.f),
	alphaPerSecond(0.3f),
	fade(true),
	scaleFactor(30.f),
	specular(true) {

	SDL_GetWindowSize(window, &width, &height);

	mainProgram = ShaderTools::compileShaders("./shaders/main.vert", "./shaders/main.frag");
	streamlineProgram = ShaderTools::compileShaders("./shaders/streamline.vert", "./shaders/streamline.frag");

	projection = glm::perspective(fovYRad, (double)width/height, near, far);

	// Default openGL state
	// If you change state you must change back to default after
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(2.f);
	glLineWidth(1.f);
	glClearColor(0.4f, 0.4f, 0.4f, 1.f);
}

// Called to render the active object. RenderEngine stores all information about how to render
void RenderEngine::render(const std::vector<const Renderable*>& objects, const glm::dmat4& view, float max, float min, float dTimeS) {
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	totalTime += dTimeS * timeMultiplier;
	totalTime = fmod(totalTime, timeRepeat);

	for (const Renderable* r : objects) {	
		
		GLuint program;
		glBindVertexArray(r->getVAO());

		if (r->getShaderType() == Shader::DEFAULT) {
			program = mainProgram;
		}
		else {
			program = streamlineProgram;
		}
		glUseProgram(program);

		glm::dmat4 modelViewD = view;

		glm::dmat4 inv = glm::inverse(modelViewD);
		glm::dvec3 eyePos = inv[3];

		modelViewD[3] = glm::dvec4(0.0, 0.0, 0.0, 1.0);

		glm::mat4 modelView = modelViewD;

		glm::vec3 eyeHigh = eyePos;
		glm::vec3 eyeLow = eyePos - (glm::dvec3)eyeHigh;


		glUniform3fv(glGetUniformLocation(program, "eyeHigh"), 1, glm::value_ptr(eyeHigh));
		glUniform3fv(glGetUniformLocation(program, "eyeLow"), 1, glm::value_ptr(eyeLow));

		glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr((glm::mat4)projection));

		glUniform1f(glGetUniformLocation(program, "altScale"), scaleFactor);
		glUniform1f(glGetUniformLocation(program, "radiusEarthM"), (float)RADIUS_EARTH_M);

		glUniform1i(glGetUniformLocation(program, "fade"), fade);
		glUniform1f(glGetUniformLocation(program, "maxDist"), max);
		glUniform1f(glGetUniformLocation(program, "minDist"), min);
		glUniform1f(glGetUniformLocation(program, "totalTime"), totalTime);
		glUniform1f(glGetUniformLocation(program, "timeMultiplier"), timeMultiplier);
		glUniform1f(glGetUniformLocation(program, "timeRepeat"), timeRepeat);
		glUniform1f(glGetUniformLocation(program, "alphaPerSecond"), alphaPerSecond);
		glUniform1f(glGetUniformLocation(program, "specularToggle"), (specular) ? 1.f : 0.f);

		r->render();
		glBindVertexArray(0);
	}
}


// Sets projection and viewport for new width and height
void RenderEngine::setWindowSize(int newWidth, int newHeight) {
	width = newWidth;
	height = newHeight;
	projection = glm::perspective(fovYRad, (double)width / height, near, far);
	glViewport(0, 0, width, height);
}


void RenderEngine::updateScaleFactor(int dir) {

	if (dir > 0) {
		scaleFactor += 1.f;
	}
	else {
		scaleFactor -= 1.f;
	}

	if (scaleFactor < 1.f) {
		scaleFactor = 1.f;
	}
}