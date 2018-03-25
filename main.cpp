#include <iostream>

#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void errorCallback_GLFW(int error, const char* description)
{
    throw std::runtime_error("Error: " + std::string(description) + " (" + std::to_string(error) + ")\n");
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void keyCallback_GLFW(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}
#pragma GCC diagnostic pop

GLFWwindow* window;

void init()
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

  glfwSetKeyCallback(window, keyCallback_GLFW);

  glfwSwapInterval(1);
}
void cleanup()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}
void mainLoop()
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }
}

int main()
{
  std::cout << "Hello." << std::endl;

  init();
  mainLoop();
  cleanup();

  std::cout << "Goodbye." << std::endl;

  return 0;
}
