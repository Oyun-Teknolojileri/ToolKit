#include "stdafx.h"
#include "Util.h"
#include "rapidxml.hpp"
#include <fstream>

void ExtractXYFromNode(void* nodev, glm::vec2& val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = (float)std::atof(attr->value());

  attr = node->first_attribute("y");
  val.y = (float)std::atof(attr->value());
}

void ExtractXYZFromNode(void* nodev, glm::vec3& val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = (float)std::atof(attr->value());

  attr = node->first_attribute("y");
  val.y = (float)std::atof(attr->value());

  attr = node->first_attribute("z");
  val.z = (float)std::atof(attr->value());
}

void ExtractXYZFromNode(void* nodev, glm::ivec3 & val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;
  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = std::atoi(attr->value());

  attr = node->first_attribute("y");
  val.y = std::atoi(attr->value());

  attr = node->first_attribute("z");
  val.z = std::atoi(attr->value());
}

void ExtractWXYZFromNode(void* nodev, glm::vec4& val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = (float)std::atof(attr->value());

  attr = node->first_attribute("y");
  val.y = (float)std::atof(attr->value());

  attr = node->first_attribute("z");
  val.z = (float)std::atof(attr->value());

  attr = node->first_attribute("w");
  val.w = (float)std::atof(attr->value());
}

void ExtractWXYZFromNode(void* nodev, glm::uvec4& val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = std::atoi(attr->value());

  attr = node->first_attribute("y");
  val.y = std::atoi(attr->value());

  attr = node->first_attribute("z");
  val.z = std::atoi(attr->value());

  attr = node->first_attribute("w");
  val.w = std::atoi(attr->value());
}

void ExtractWXYZFromNode(void* nodev, glm::ivec4 & val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

  rapidxml::xml_attribute<>* attr = node->first_attribute("x");
  val.x = std::atoi(attr->value());

  attr = node->first_attribute("y");
  val.y = std::atoi(attr->value());

  attr = node->first_attribute("z");
  val.z = std::atoi(attr->value());

  attr = node->first_attribute("w");
  val.w = std::atoi(attr->value());
}

void ExtractQuatFromNode(void* nodev, glm::quat& val)
{
  rapidxml::xml_node<>* node = (rapidxml::xml_node<>*) nodev;

  glm::vec4 tmp;
  ExtractWXYZFromNode(node, tmp);
  val = glm::quat(tmp.w, tmp.xyz);
}

bool CheckFile(std::string path)
{
  std::ifstream f(path.c_str());
  return f.good();
}
