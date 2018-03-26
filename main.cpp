#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


std::string readFile(std::string filePath)
{
  std::string ret = "";
  std::string tmp;
  std::ifstream file;
  file.open(filePath);
  if(!file.is_open())
  {
  	throw std::runtime_error("Failed to open file: " + filePath);
  }
  while (std::getline(file, tmp))
  {
    ret += tmp + "\n";
  }
  file.close();
  return ret;
}

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

int main( void )
{
	glfwSetErrorCallback(errorCallback_GLFW);
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW.\n");
  }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenglTest", NULL, NULL);
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

	glClearColor(0.1f, 0.2f, 0.5f, 1.0f);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	GLuint programID = compileShaders( "shader.vert", "shader.frag" );

	glm::dvec3 vertices[] = {
		{-1.0, -1.0, 0.0},
		{ 1.0, -1.0, 0.0},
		{ 0.0,  1.0, 0.0},
	};

	GLuint vertexBufferID;
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribLPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_DOUBLE,          // type
		0,                  // stride
		(void*)0            // array buffer offset
	);

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

    glm::mat4 modelToWorld = glm::rotate(glm::mat4(), glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
    modelToWorld = glm::translate(glm::mat4(), {0.0, 0.0, -5.0}) * modelToWorld;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glm::mat4 worldToProjection = glm::perspective(glm::radians(60.0f), GLfloat(width)/GLfloat(height), 0.1f, 10.0f);

    glUniformMatrix4fv(0, 1, GL_FALSE, &(modelToWorld[0][0]));
    glUniformMatrix4fv(1, 1, GL_FALSE, &(worldToProjection[0][0]));

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glDeleteBuffers(1, &vertexBufferID);
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteProgram(programID);

	glfwTerminate();

	return 0;
}
