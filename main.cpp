#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "readWrite.hpp"
#include "loadObj.hpp"
#include "lodepng.hpp"

struct Texture
{
	Texture(uint32_t width, uint32_t height, std::vector<unsigned char> image) :
		width(width), height(height), image(image)
	{}
	uint32_t width, height;
	std::vector<unsigned char> image;
};
Texture defaultTexture = Texture(1, 1, {255, 255, 255, 255});
Texture defaultNormalMap = Texture(1, 1, {128, 128, 255, 255});

GLuint programID;
GLFWwindow* window;

glm::vec3 cameraPosition = {0.0, 10.0, 10.0};
glm::vec3 cameraViewDirection = {0.0, -0.5, -1.0};
glm::vec3 cameraUp = {0.0, 1.0, 0.0};

glm::vec3 lightPosition = {0.0, 90.0, -20.0};

GLuint textureID;
GLuint normalMapID;

GLuint renderedTextureID;
GLuint renderToFramebufferID;
GLuint depthRenderbufferID;

GLuint depthMapProgramID;
GLuint depthMapID;
GLuint depthMapFramebufferID;
const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;

GLuint compileShaders(std::string vertFile, std::string fragFile)
{
	GLuint programID;
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	std::string vertexShaderCode = readFile(vertFile);
  const char* vertAdapter = vertexShaderCode.data();
  glShaderSource(vertexShaderID, 1, &vertAdapter, 0);
  glCompileShader(vertexShaderID);

  std::string fragmentShaderCode = readFile(fragFile);
  const char* fragAdapter = fragmentShaderCode.data();
  glShaderSource(fragmentShaderID, 1, &fragAdapter, 0);
  glCompileShader(fragmentShaderID);

  programID = glCreateProgram();
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);
  glLinkProgram(programID);

  GLint success = 0;
  glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE)
  {
    GLint maxLength = 0;
	  glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);

    glGetShaderInfoLog(fragmentShaderID, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data() << std::endl;

    glDeleteShader(fragmentShaderID);
    throw std::runtime_error("Failed to compile fragment shader.\n");
  }
	glDeleteShader(fragmentShaderID);
  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE)
  {
    GLint maxLength = 0;
	  glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(vertexShaderID, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data() << std::endl;

    glDeleteShader(vertexShaderID);
    throw std::runtime_error("Failed to compile vertex shader.\n");
  }
	glDeleteShader(vertexShaderID);
	return programID;
}

Texture generateTexture(const char* filePath)
{
	std::vector<unsigned char> image;
  unsigned int width, height;
  auto error = lodepng::decode(image, width, height, filePath);
  if(error)
	{
		throw std::runtime_error("decoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
	}

	return Texture(width, height, image);
}

GLuint loadTexture(Texture texture)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.image.data());
	//enable mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.3f);
	return textureID;
}

void renderTexture(GLuint textureID)
{
	GLuint textureRenderProgramID = compileShaders("shader_render_texture.vert", "shader_render_texture.frag");

	glm::vec2 vertices[] =
	{
	  glm::vec2(-1.0, 1.0),
		glm::vec2(0.0, 0.0),

		glm::vec2(-1.0, -1.0),
		glm::vec2(0.0, 1.0),

		glm::vec2(1.0, -1.0),
		glm::vec2(1.0, 1.0),


	  glm::vec2(1.0, -1.0),
		glm::vec2(1.0, 1.0),

		glm::vec2(1.0, 1.0),
		glm::vec2(1.0, 0.0),

		glm::vec2(-1.0, 1.0),
		glm::vec2(0.0, 0.0),
	};

	glUseProgram(textureRenderProgramID);

	GLuint bufferID;
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) , vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,     																		// size
		GL_FLOAT,          												// type
		GL_FALSE,																	// normalized
		sizeof(glm::vec2)*2,											// stride
		(void*)0            											// array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,     																		// size
		GL_FLOAT,          												// type
		GL_FALSE,																	// normalized
		sizeof(glm::vec2)*2,											// stride
		(void*)sizeof(glm::vec2)            											// array buffer offset
	);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programID, "texture"), 0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteBuffers(1, &bufferID);
	glDeleteProgram(textureRenderProgramID);
}

void errorCallback_GLFW(int error, const char* description)
{
    throw std::runtime_error("Error: " + std::string(description) + " (" + std::to_string(error) + ")\n");
}

struct Entity
{
	Model3D model;
	std::vector<GLuint> vertexBufferIDs;
	std::vector<GLuint> vertexArrayObjectIDs;

	glm::vec3 position;

	Entity(Model3D model, glm::vec3 position) : model(model), position(position)
	{
		vertexBufferIDs = std::vector<GLuint>();
		vertexArrayObjectIDs = std::vector<GLuint>();
		for(auto& object : model.objects)
		{
			vertexArrayObjectIDs.emplace_back();
			glGenVertexArrays(1, &vertexArrayObjectIDs.back());
			glBindVertexArray(vertexArrayObjectIDs.back());

			vertexBufferIDs.emplace_back();
			glGenBuffers(1, &vertexBufferIDs.back());
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIDs.back());
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * object.vertices.size() , object.vertices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, position)   			// array buffer offset
			);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, normal)						// array buffer offset
			);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, tangent)					// array buffer offset
			);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(
				3,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
				2,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, textureCoordinate)// array buffer offset
			);
		}
	}

	~Entity()
	{
		for(size_t i = 0; i< model.objects.size(); i++)
		{
			glDeleteBuffers(1, &vertexBufferIDs[i]);
			glDeleteVertexArrays(1, &vertexArrayObjectIDs[i]);
		}
	}

	void render()
	{
		for(size_t i = 0; i< model.objects.size(); i++)
		{

			glBindVertexArray(vertexArrayObjectIDs[i]);

			glUseProgram(programID);

			glm::mat4 modelRotation = glm::rotate(glm::mat4(), glm::radians(100.0f), glm::vec3(0.0, 1.0, 1.0));
			glm::mat4 modelTranslation = glm::translate(glm::mat4(), position);

			glm::mat4 modelToWorld = modelTranslation * modelRotation;

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glm::mat4 worldToProjection =
				glm::perspective(glm::radians(60.0f), GLfloat(width)/GLfloat(height), 0.1f, 100.0f) *
				glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);


			glm::mat4 worldToLightSpace =
				glm::perspective(glm::radians(90.0f), GLfloat(SHADOW_WIDTH)/GLfloat(SHADOW_HEIGHT), 0.1f, 100.0f) * //glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f) *
				glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));


			glUniformMatrix4fv(
				glGetUniformLocation(programID, "modelToWorld"),
				1, GL_FALSE, &(modelToWorld[0][0])
			);
			glUniformMatrix4fv(
				glGetUniformLocation(programID, "worldToProjection"),
				1, GL_FALSE, &(worldToProjection[0][0])
			);
			glUniformMatrix4fv(
				glGetUniformLocation(programID, "worldToLightSpace"),
				1, GL_FALSE, &(worldToLightSpace[0][0])
			);
			glUniform3fv(
				glGetUniformLocation(programID, "cameraPosition"),
				1, &cameraPosition[0]
			);
			glUniform3fv(
				glGetUniformLocation(programID, "lightPosition"),
				1, &lightPosition[0]
			);
			glUniform3fv(
				glGetUniformLocation(programID, "ambientColor"),
				1, &model.objects[i].material.ambientColor[0]
			);
			glUniform3fv(
				glGetUniformLocation(programID, "diffuseColor"),
				1, &model.objects[i].material.diffuseColor[0]
			);
			glUniform3fv(
				glGetUniformLocation(programID, "specularColor"),
				1, &model.objects[i].material.specularColor[0]
			);
			glUniform1f(
				glGetUniformLocation(programID, "transparency"),
				model.objects[i].material.transparency
			);
			glUniform1f(
				glGetUniformLocation(programID, "shininess"),
				model.objects[i].material.shininess
			);

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(programID, "texture"), 0);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glActiveTexture(GL_TEXTURE1);
			glUniform1i(glGetUniformLocation(programID, "normalMap"), 1);
			glBindTexture(GL_TEXTURE_2D, normalMapID);

			glActiveTexture(GL_TEXTURE2);
			glUniform1i(glGetUniformLocation(programID, "depthMap"), 2);
			glBindTexture(GL_TEXTURE_2D, depthMapID);

			glDrawArrays(GL_TRIANGLES, 0, model.objects[i].vertices.size());
		}
	}
	void renderDepthMap()
	{
		for(size_t i = 0; i< model.objects.size(); i++)
		{

			glBindVertexArray(vertexArrayObjectIDs[i]);

			glUseProgram(depthMapProgramID);

			glm::mat4 modelRotation = glm::rotate(glm::mat4(), glm::radians(100.0f), glm::vec3(0.0, 1.0, 1.0));
			glm::mat4 modelTranslation = glm::translate(glm::mat4(), position);

			glm::mat4 modelToWorld = modelTranslation * modelRotation;

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glm::mat4 worldToProjection =
				glm::perspective(glm::radians(90.0f), GLfloat(SHADOW_WIDTH)/GLfloat(SHADOW_HEIGHT), 0.1f, 100.0f) * //glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f) *
				glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));

				glUniformMatrix4fv(
					glGetUniformLocation(programID, "modelToWorld"),
					1, GL_FALSE, &(modelToWorld[0][0])
				);
				glUniformMatrix4fv(
					glGetUniformLocation(programID, "worldToProjection"),
					1, GL_FALSE, &(worldToProjection[0][0])
				);

			glDrawArrays(GL_TRIANGLES, 0, model.objects[i].vertices.size());
		}
	}
};

int main( void )
{

	glfwSetErrorCallback(errorCallback_GLFW);
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW.\n");
  }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	window = glfwCreateWindow(800, 600, "OpenglTest", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to initialize Window or context.\n");
  }
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    throw std::runtime_error("Failed to initialize GLEW: "+ std::string((char*)glewGetErrorString(err)) + "\n");
  }
  if(!(glewIsSupported("GL_ARB_vertex_attrib_64bit") && glewIsSupported("GL_ARB_gpu_shader_fp64")))
  {
    throw std::runtime_error("Failed to find required extensions.\n");
  }
	glfwSwapInterval(1);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);

	glClearColor(0.1f, 0.2f, 0.4f, 1.0f);

	programID = compileShaders("shader.vert", "shader.frag");

	textureID = loadTexture(defaultTexture);
	normalMapID = loadTexture(generateTexture("normalMap.png"));//defaultNormalMap);

	Entity e = Entity(loadObj("spaceboat.obj"), {0.0, 4.0, -20.0});

	Entity p = Entity(loadObj("Plane.obj"), {0.0, -3.0, -20.0});

	depthMapProgramID = compileShaders("shader_shadow.vert", "shader_shadow.frag");

	glGenFramebuffers(1, &depthMapFramebufferID);

	glGenTextures(1, &depthMapID);
	glBindTexture(GL_TEXTURE_2D, depthMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFramebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFramebufferID);
  glClear(GL_DEPTH_BUFFER_BIT);
  e.renderDepthMap();
  p.renderDepthMap();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while(!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		e.render();
		p.render();

		//renderTexture(depthMapID);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glDeleteProgram(programID);

	glfwTerminate();

	return 0;
}
