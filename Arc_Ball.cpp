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

// �f�B���N�g���̐ݒ�t�@�C��
#include "common.h"

void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
	// �N���b�N�����{�^���ŏ�����؂�ւ���
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		arcballMode = ARCBALL_MODE_ROTATE;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		arcballMode = ARCBALL_MODE_SCALE;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		arcballMode = ARCBALL_MODE_TRANSLATE;
	}

	// �N���b�N���ꂽ�ʒu���擾
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

// �X�N���[����̈ʒu���A�[�N�{�[������̈ʒu�ɕϊ�����֐�
glm::vec3 getVector(double x, double y) {
	glm::vec3 pt(2.0 * x / WIN_WIDTH - 1.0,
		-2.0 * y / WIN_HEIGHT + 1.0, 0.0);

	const double xySquared = pt.x * pt.x + pt.y * pt.y;
	if (xySquared <= 1.0) {
		// �P�ʉ~�̓����Ȃ�z���W���v�Z
		pt.z = std::sqrt(1.0 - xySquared);
	}
	else {
		// �O���Ȃ狅�̊O�g��ɂ���ƍl����
		pt = glm::normalize(pt);
	}

	return pt;
}

void updateRotate() {
	static const double Pi = 4.0 * std::atan(1.0);

	// �X�N���[�����W���A�[�N�{�[������̍��W�ɕϊ�
	const glm::vec3 u = glm::normalize(getVector(newPos.x, newPos.y));
	const glm::vec3 v = glm::normalize(getVector(oldPos.x, oldPos.y));

	// �J�������W�ɂ������]�� (=�I�u�W�F�N�g���W�ɂ������]��)
	const double angle = std::acos(std::max(-1.0f, std::min(glm::dot(u, v), 1.0f)));

	// �J������Ԃɂ������]��
	const glm::vec3 rotAxis = glm::cross(v, u);

	// �J�������W�̏������[���h���W�ɕϊ�����s��
	const glm::mat4 c2oMat = glm::inverse(viewMat * modelMat);

	// �I�u�W�F�N�g���W�ɂ������]��
	const glm::vec3 rotAxisObjSpace = glm::vec3(c2oMat * glm::vec4(rotAxis, 0.0f));

	// ��]�s��̍X�V
	acRotMat = glm::rotate((float)(4.0 * angle), rotAxisObjSpace) * acRotMat;
}

void updateTranslate() {
	// �I�u�W�F�N�g�d�S�̃X�N���[�����W�����߂�
	glm::vec4 gravityScreenSpace = (projMat * viewMat * modelMat) * glm::vec4(gravity.x, gravity.y, gravity.z, 1.0f);
	gravityScreenSpace /= gravityScreenSpace.w;

	// �X�N���[�����W�n�ɂ�����ړ���
	glm::vec4 newPosScreenSpace(2.0 * newPos.x / WIN_WIDTH, -2.0 * newPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);
	glm::vec4 oldPosScreenSpace(2.0 * oldPos.x / WIN_WIDTH, -2.0 * oldPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);

	// �X�N���[�����W�̏����I�u�W�F�N�g���W�ɕϊ�����s��
	const glm::mat4 s2oMat = glm::inverse(projMat * viewMat * modelMat);

	// �X�N���[����Ԃ̍��W���I�u�W�F�N�g��Ԃɕϊ�
	glm::vec4 newPosObjSpace = s2oMat * newPosScreenSpace;
	glm::vec4 oldPosObjSpace = s2oMat * oldPosScreenSpace;
	newPosObjSpace /= newPosObjSpace.w;
	oldPosObjSpace /= oldPosObjSpace.w;

	// �I�u�W�F�N�g���W�n�ł̈ړ���
	const glm::vec3 transObjSpace = glm::vec3(newPosObjSpace - oldPosObjSpace);

	// �I�u�W�F�N�g��Ԃł̕��s�ړ�
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
		// �}�E�X�̌��݈ʒu���X�V
		newPos = glm::ivec2(xpos, ypos);

		// �}�E�X�����܂蓮���Ă��Ȃ����͏��������Ȃ�
		const double dx = newPos.x - oldPos.x;
		const double dy = newPos.y - oldPos.y;
		const double length = dx * dx + dy * dy;
		if (length < 2.0f * 2.0f) {
			return;
		}
		else {
			updateMouse(); //�}�E�X�̈ʒu���X�V
			oldPos = glm::ivec2(xpos, ypos);
		}
	}
}

void wheelEvent(GLFWwindow *window, double xpos, double ypos) {
	acScale += ypos / 10.0;
	updateScale();
}