#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "vertex.hpp"
#include "readWrite.hpp"

struct Material
{
  glm::vec3 ambientColor;
  glm::vec3 diffuseColor;
  glm::vec3 specularColor;
  glm::float_t transparency;
  glm::float_t shininess;
};

struct Object3D
{
  std::vector<Vertex> vertices = std::vector<Vertex>();
  Material material;
};

struct Model3D
{
  std::vector<Object3D> objects = std::vector<Object3D>();
};

void loadMtl(std::string filePath, std::vector<Material>& materials, std::map<std::string, size_t>& materialIndices)
{
  std::string mtlData = readFile(filePath);

  std::stringstream modelDataStream(mtlData);
	std::string bufferString;
	while (getline(modelDataStream, bufferString))
	{
		std::stringstream bufferStringStream(bufferString);
		bufferStringStream >> bufferString;

    if(bufferString == "newmtl")
    {
      bufferStringStream >> bufferString;
      materialIndices[bufferString] = materials.size();
      materials.emplace_back();
    }
    else if(bufferString == "Ka")
    {
      bufferStringStream >> bufferString;
      materials.back().ambientColor.r = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().ambientColor.g = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().ambientColor.b = std::atof(bufferString.c_str());
    }
    else if(bufferString == "Kd")
    {
      bufferStringStream >> bufferString;
      materials.back().diffuseColor.r = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().diffuseColor.g = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().diffuseColor.b = std::atof(bufferString.c_str());
    }
    else if(bufferString == "Ks")
    {
      bufferStringStream >> bufferString;
      materials.back().specularColor.r = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().specularColor.g = std::atof(bufferString.c_str());
      bufferStringStream >> bufferString;
      materials.back().specularColor.b = std::atof(bufferString.c_str());
    }
    else if(bufferString == "d" || bufferString == "Tr")
    {
      bufferStringStream >> bufferString;
      materials.back().transparency = std::atof(bufferString.c_str());
    }
    else if(bufferString == "Ns")
    {
      bufferStringStream >> bufferString;
      materials.back().shininess = std::atof(bufferString.c_str());
    }
  }
}

Model3D loadObj(std::string filePath)
{
  std::string modelData = readFile(filePath);

  std::vector<glm::vec3> positions = std::vector<glm::vec3>();
  std::vector<glm::vec2> textureCoordinates = std::vector<glm::vec2>();
  std::vector<glm::vec3> normals = std::vector<glm::vec3>();

  std::vector<Vertex> faces = std::vector<Vertex>();

  std::vector<Material> materials = std::vector<Material>();
  std::map<std::string, size_t> materialIndices = std::map<std::string, size_t>();

  Model3D ret = Model3D();

  std::stringstream modelDataStream(modelData);
	std::string bufferString;
	while (getline(modelDataStream, bufferString))
	{
		std::stringstream bufferStringStream(bufferString);
		bufferStringStream >> bufferString;
		if (bufferString == "o")
		{
      ret.objects.push_back(Object3D());
		}
		else if (bufferString == "mtllib")
		{
		  bufferStringStream >> bufferString;
      loadMtl(bufferString, materials, materialIndices);
		}
		else if (bufferString == "usemtl")
		{
		  bufferStringStream >> bufferString;
      if(materialIndices.find(bufferString) != materialIndices.end())
      {
        ret.objects.back().material = materials[materialIndices[bufferString]];
      }
		}
		else if (bufferString == "v")
		{
  		glm::vec3 tempVec;
  		bufferStringStream >> bufferString;
  		tempVec.x = std::stof(bufferString);
  		bufferStringStream >> bufferString;
  		tempVec.y = std::stof(bufferString);
  		bufferStringStream >> bufferString;
  		tempVec.z = std::stof(bufferString);
  		positions.push_back(tempVec);
		}
		else if (bufferString == "vt")
		{
			glm::vec2 tempVec;
			bufferStringStream >> bufferString;
			tempVec.x = std::stof(bufferString);
			bufferStringStream >> bufferString;
			tempVec.y = 1.0f - std::stof(bufferString);
			textureCoordinates.push_back(tempVec);
		}
		else if (bufferString == "vn")
		{
			glm::vec3 tempVec;
			bufferStringStream >> bufferString;
			tempVec.x = std::stof(bufferString);
			bufferStringStream >> bufferString;
			tempVec.y = std::stof(bufferString);
			bufferStringStream >> bufferString;
			tempVec.z = std::stof(bufferString);
			normals.push_back(tempVec);
		}
		else if (bufferString == "f")
		{
      while (bufferStringStream >> bufferString)
      {
        Vertex tempVertex;
        std::stringstream tempStream(bufferString);
        std::string tempBufferString;
        int iterator = 0;
        while (std::getline(tempStream, tempBufferString, '/'))
        {
          if (tempBufferString == "")
          {
            if (iterator == 0)
            {
              tempVertex.position = glm::vec3(0.0, 0.0, 0.0);
            }
            else if (iterator == 1)
            {
              tempVertex.textureCoordinate = glm::vec2(0.0, 1.0);
            }
            else if (iterator == 2)
            {
              tempVertex.normal = glm::vec3(0.0, 1.0, 0.0);
            }
          }
          else if (iterator == 0)
          {
            tempVertex.position = positions[stoi(tempBufferString) - 1];
          }
          else if (iterator == 1)
          {
            tempVertex.textureCoordinate = textureCoordinates[stoi(tempBufferString) - 1];
          }
          else if (iterator == 2)
          {
            tempVertex.normal = normals[stoi(tempBufferString) - 1];
          }
          iterator++;
        }
        ret.objects.back().vertices.push_back(tempVertex);
      }
    }
	}
  return ret;
}
