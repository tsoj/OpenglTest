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

GLuint programID;
GLFWwindow* window;

glm::vec3 cameraPosition = {0.0, 0.0, 0.0};
glm::vec3 cameraViewDirection = {0.0, 0.0, -1.0};
glm::vec3 cameraUp = {0.0, 1.0, 0.0};

glm::vec3 lightPosition = {0.0, 20.0, 0.0};

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
	//std::vector<GLuint> programIDs;

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
				(void*)0            											// array buffer offset
			);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)(sizeof(glm::vec3))					// array buffer offset
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

			glm::mat4 modelRotation = glm::rotate(glm::mat4(), glm::radians(100.0f), glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 modelTranslation = glm::translate(glm::mat4(), position);

			glm::mat4 modelToWorld = modelTranslation * modelRotation;

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glm::mat4 worldToProjection =
				glm::perspective(glm::radians(60.0f), GLfloat(width)/GLfloat(height), 0.1f, 100.0f) *
				glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);

			glUniformMatrix4fv(0, 1, GL_FALSE, &(modelToWorld[0][0]));
			glUniformMatrix4fv(1, 1, GL_FALSE, &(worldToProjection[0][0]));
			glUniformMatrix4fv(2, 1, GL_FALSE, &(modelRotation[0][0]));

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

	glClearColor(0.02f, 0.04f, 0.1f, 1.0f);

	programID = compileShaders("shader.vert", "shader.frag");

	Entity e = Entity(loadObj("spaceboat.obj"), {0.0, 1.0, -20.0});

	Entity p = Entity(loadObj("Plane.obj"), {0.0, -3.0, -20.0});

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		e.render();
		p.render();

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glDeleteProgram(programID);

	glfwTerminate();

	return 0;
}
