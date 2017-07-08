#pragma once

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

#define GLFW_INCLUDE_GLU
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

void mouseEvent(GLFWwindow *window, int button, int action, int mods);

// スクリーン上の位置をアークボール球上の位置に変換する関数
glm::vec3 getVector(double x, double y);

void updateRotate();

void updateTranslate();

void updateScale();

void updateMouse();

void mouseMoveEvent(GLFWwindow *window, double xpos, double ypos);

void wheelEvent(GLFWwindow *window, double xpos, double ypos);