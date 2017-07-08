#include "Function.h"

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

// ディレクトリの設定ファイル
#include "common.h"

void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
	// クリックしたボタンで処理を切り替える
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		arcballMode = ARCBALL_MODE_ROTATE;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		arcballMode = ARCBALL_MODE_SCALE;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		arcballMode = ARCBALL_MODE_TRANSLATE;
	}

	// クリックされた位置を取得
	double px, py;
	glfwGetCursorPos(window, &px, &py);

	if (action == GLFW_PRESS) {
		if (!isDragging) {
			isDragging = true;
			oldPos = glm::ivec2(px, py);
			newPos = glm::ivec2(px, py);
		}
	}
	else {
		isDragging = false;
		oldPos = glm::ivec2(0, 0);
		newPos = glm::ivec2(0, 0);
		arcballMode = ARCBALL_MODE_NONE;
	}
}

// スクリーン上の位置をアークボール球上の位置に変換する関数
glm::vec3 getVector(double x, double y) {
	glm::vec3 pt(2.0 * x / WIN_WIDTH - 1.0,
		-2.0 * y / WIN_HEIGHT + 1.0, 0.0);

	const double xySquared = pt.x * pt.x + pt.y * pt.y;
	if (xySquared <= 1.0) {
		// 単位円の内側ならz座標を計算
		pt.z = std::sqrt(1.0 - xySquared);
	}
	else {
		// 外側なら球の外枠上にあると考える
		pt = glm::normalize(pt);
	}

	return pt;
}

void updateRotate() {
	static const double Pi = 4.0 * std::atan(1.0);

	// スクリーン座標をアークボール球上の座標に変換
	const glm::vec3 u = glm::normalize(getVector(newPos.x, newPos.y));
	const glm::vec3 v = glm::normalize(getVector(oldPos.x, oldPos.y));

	// カメラ座標における回転量 (=オブジェクト座標における回転量)
	const double angle = std::acos(std::max(-1.0f, std::min(glm::dot(u, v), 1.0f)));

	// カメラ空間における回転軸
	const glm::vec3 rotAxis = glm::cross(v, u);

	// カメラ座標の情報をワールド座標に変換する行列
	const glm::mat4 c2oMat = glm::inverse(viewMat * modelMat);

	// オブジェクト座標における回転軸
	const glm::vec3 rotAxisObjSpace = glm::vec3(c2oMat * glm::vec4(rotAxis, 0.0f));

	// 回転行列の更新
	acRotMat = glm::rotate((float)(4.0 * angle), rotAxisObjSpace) * acRotMat;
}

void updateTranslate() {
	// オブジェクト重心のスクリーン座標を求める
	glm::vec4 gravityScreenSpace = (projMat * viewMat * modelMat) * glm::vec4(gravity.x, gravity.y, gravity.z, 1.0f);
	gravityScreenSpace /= gravityScreenSpace.w;

	// スクリーン座標系における移動量
	glm::vec4 newPosScreenSpace(2.0 * newPos.x / WIN_WIDTH, -2.0 * newPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);
	glm::vec4 oldPosScreenSpace(2.0 * oldPos.x / WIN_WIDTH, -2.0 * oldPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);

	// スクリーン座標の情報をオブジェクト座標に変換する行列
	const glm::mat4 s2oMat = glm::inverse(projMat * viewMat * modelMat);

	// スクリーン空間の座標をオブジェクト空間に変換
	glm::vec4 newPosObjSpace = s2oMat * newPosScreenSpace;
	glm::vec4 oldPosObjSpace = s2oMat * oldPosScreenSpace;
	newPosObjSpace /= newPosObjSpace.w;
	oldPosObjSpace /= oldPosObjSpace.w;

	// オブジェクト座標系での移動量
	const glm::vec3 transObjSpace = glm::vec3(newPosObjSpace - oldPosObjSpace);

	// オブジェクト空間での平行移動
	acTransMat = glm::translate(acTransMat, transObjSpace);
}

void updateScale() {
	acScaleMat = glm::scale(glm::vec3(acScale, acScale, acScale));
}

void updateMouse() {
	switch (arcballMode) {
	case ARCBALL_MODE_ROTATE:
		updateRotate();
		break;

	case ARCBALL_MODE_TRANSLATE:
		updateTranslate();
		break;

	case ARCBALL_MODE_SCALE:
		acScale += (float)(oldPos.y - newPos.y) / WIN_HEIGHT;
		updateScale();
		break;
	}
}

void mouseMoveEvent(GLFWwindow *window, double xpos, double ypos) {
	if (isDragging) {
		// マウスの現在位置を更新
		newPos = glm::ivec2(xpos, ypos);

		// マウスがあまり動いていない時は処理をしない
		const double dx = newPos.x - oldPos.x;
		const double dy = newPos.y - oldPos.y;
		const double length = dx * dx + dy * dy;
		if (length < 2.0f * 2.0f) {
			return;
		}
		else {
			updateMouse(); //マウスの位置を更新
			oldPos = glm::ivec2(xpos, ypos);
		}
	}
}

void wheelEvent(GLFWwindow *window, double xpos, double ypos) {
	acScale += ypos / 10.0;
	updateScale();
}