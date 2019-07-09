#include "RenderEngine.h"

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Conversions.h"
#include "Renderable.h"
#include "ShaderTools.h"
#include "Window.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <iostream>


// Dear ImGUI window. Allows chaning render parameters
void RenderEngine::ImGui() {
	if (ImGui::CollapsingHeader("Render params")) {
		ImGui::Checkbox("Specular highlights", &specular);
		ImGui::Checkbox("Diffuse", &diffuse);
		ImGui::SliderFloat("Line width", &lineWidth, 0.1f, 10.f);
		ImGui::SliderFloat("Outline width", &outlineWidth, 0.1f, 10.f);
		ImGui::Checkbox("Pause time", &pause);
		ImGui::InputFloat("Time scale factor", &timeMultiplier, 100.f, 1000.f);
		ImGui::InputFloat("Time repeat interval", &timeRepeat, 100.f, 1000.f);
		ImGui::SliderFloat("Alpha multiplier/s", &alphaPerSecond, 0.f, 1.f);
		ImGui::SliderFloat("Altitude scale factor", &scaleFactor, 1.f, 100.f);
	}
}


// Sets up for rendering to the provided window with provided viewport parameters
//
// window - window to render to
// x - left viewport x
// y - lower viewport y
// width - viewport width
// height - viewport height
// cameraDist - distance camera is from surface of Earth
RenderEngine::RenderEngine(const Window& window, int x, int y, int width, int height, double cameraDist) :
	window(window),
	x(x),
	y(y),
	width(width),
	height(height),
	fovYRad(60.0 * (M_PI / 180.0)),
	totalTime(0.f),
	timeMultiplier(30000.f),
	timeRepeat(100000.f),
	alphaPerSecond(0.3f),
	pause(false),
	specular(false),
	diffuse(true),
	lineWidth(1.f),
	outlineWidth(1.f),
	scaleFactor(10.f) {

	// Compile shaders
	// TODO this could be static - does not need to be done for each engine
	mainProgram = ShaderTools::compileShaders("./shaders/main.vert", "./shaders/main.frag");
	streamlineProgram = ShaderTools::compileShaders("./shaders/streamline.vert", "./shaders/streamline.frag");
	streamlineProgram2 = ShaderTools::compileShaders("./shaders/streamline2.vert", "./shaders/streamline2.frag");

	updatePlanes(cameraDist);

	// Default openGL state
	// If you change state you must change back to default after
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(2.f);
	glLineWidth(lineWidth);
	glClearColor(1.f, 1.f, 1.f, 1.f);
}


// Sets up for rendering to the provided window for the full window
//
// window - window to render to
// cameraDist - distance camera is from surface of Earth
RenderEngine::RenderEngine(const Window& window, double cameraDist) :
	RenderEngine(window, 0, 0, window.getWidth(), window.getHeight(), cameraDist) {}


void RenderEngine::clearViewport() {

	glEnable(GL_SCISSOR_TEST);
	glScissor(x - 1, y - 1, width + 2, height + 2);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(1.f, 1.f, 1.f, 1.f);

	glDisable(GL_SCISSOR_TEST);
}


// Render provided object. Render engine stores all information about how to render
//
// objects - list of renderables to render
// view - view matrix
// dTimeS - time since last render in seconds
void RenderEngine::render(const std::vector<Renderable*>& objects, const glm::dmat4& view, float dTimeS) {
	
	glViewport(x, y, width, height);
	if (!pause) {
		totalTime += dTimeS * timeMultiplier;
		totalTime = fmod(totalTime, timeRepeat);
	}

	int numPasses = (outlineWidth > lineWidth) ? 2 : 1;

	for (int pass = 1; pass <= numPasses; pass++) {

		if (pass == 1) {
			glLineWidth(lineWidth);
		}
		else {
			glLineWidth(outlineWidth);
		}

		for (size_t i = objects.size(); i >= 1; i--) {

			Renderable* r = objects[i - 1];
		
			GLuint program;
			if (r->getVAO() == -1) {
				r->assignBuffers();
				r->setBufferData();
			}
			glBindVertexArray(r->getVAO());

			if (r->getShaderType() == Shader::DEFAULT && pass == 1) {
				program = mainProgram;
			}
			else if (r->getShaderType() == Shader::DEFAULT && pass == 2){
				continue;
			}
			else if (pass == 1) {
				program = streamlineProgram;
			}
			else {
				program = streamlineProgram2;
			}
			glUseProgram(program);

			// Get eye position from view matrix
			glm::dmat4 modelViewD = view;

			glm::dmat4 inv = glm::inverse(modelViewD);
			glm::dvec3 eyePos = inv[3];

			modelViewD[3] = glm::dvec4(0.0, 0.0, 0.0, 1.0);

			glm::mat4 modelView = modelViewD;

			glm::vec3 eyeHigh = eyePos;
			glm::vec3 eyeLow = eyePos - (glm::dvec3)eyeHigh;

			// Set uniforms
			glUniform3fv(glGetUniformLocation(program, "eyeHigh"), 1, glm::value_ptr(eyeHigh));
			glUniform3fv(glGetUniformLocation(program, "eyeLow"), 1, glm::value_ptr(eyeLow));

			glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
			glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr((glm::mat4)projection));

			glUniform1f(glGetUniformLocation(program, "altScale"), scaleFactor);
			glUniform1f(glGetUniformLocation(program, "radiusEarthM"), (float)RADIUS_EARTH_M);

			glUniform1f(glGetUniformLocation(program, "totalTime"), totalTime);
			glUniform1f(glGetUniformLocation(program, "timeMultiplier"), timeMultiplier);
			glUniform1f(glGetUniformLocation(program, "timeRepeat"), timeRepeat);
			glUniform1f(glGetUniformLocation(program, "alphaPerSecond"), alphaPerSecond);
			glUniform1f(glGetUniformLocation(program, "specularToggle"), (specular) ? 1.f : 0.f);
			glUniform1f(glGetUniformLocation(program, "diffuseToggle"), (diffuse) ? 0.f : 1.f);

			r->render();
			glBindVertexArray(0);
		}
	}
}


// Sets projection and viewport for new width and height
//
// newWidth - new window width
// newHeight - new window height
void RenderEngine::setWindowSize(int newWidth, int newHeight) {
	width = newWidth;
	height = newHeight;
	projection = glm::perspective(fovYRad, (double)width / height, near, far);
	glViewport(0, 0, width, height);
}


// Update altitude scale factor
//
// dir - direction of change, possitive or negative
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


// Updates near and far planes based on camera distance
//
// cameraDist - distance camera is from surface of the Earth
void RenderEngine::updatePlanes(double cameraDist) {

	double cameraPlusRad = RADIUS_EARTH_M + cameraDist;
	double t = sqrt(cameraPlusRad * cameraPlusRad + RADIUS_EARTH_M * RADIUS_EARTH_M);
	double theta = asin(RADIUS_EARTH_M / cameraPlusRad);

	far = cos(theta) * t;
	// TODO maybe a better way to calculate near
	near = cameraDist - RADIUS_EARTH_M - 33000.0 * scaleFactor;
	//if (near < 0.0) {
	//	near = cameraDist - 33000.0 * scaleFactor;
	//}
	if (near < 0.0) {
		if (far < 6500000.0) {
			near = far / 1000.0;
		}
		else {
			near = far / 100.0;
		}

	}

	projection = glm::perspective(fovYRad, (double)width / height, near, far);
}