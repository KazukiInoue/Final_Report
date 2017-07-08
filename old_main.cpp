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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// �f�B���N�g���̐ݒ�t�@�C��
#include "common.h"

static int WIN_WIDTH = 500;                       // �E�B���h�E�̕�
static int WIN_HEIGHT = 500;                       // �E�B���h�E�̍���
static const char *WIN_TITLE = "OpenGL Course";     // �E�B���h�E�̃^�C�g��

static const double PI = 4.0 * std::atan(1.0);

// ���_�I�u�W�F�N�g
struct Vertex {
	Vertex(const glm::vec3 &position_, const glm::vec3 &color_)
		: position(position_)
		, color(color_)
	{
	}

	glm::vec3 position;
	glm::vec3 color;
};

static const glm::vec3 cube_positions[8] = {
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f,  1.0f,  1.0f),
	glm::vec3(1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f,  1.0f,  1.0f)
};

static const glm::vec3 background_positions[4] = {
	glm::vec3(-10.0f, -2.0f, -10.0f),
	glm::vec3(10.0f, -2.0f, -10.0f),
	glm::vec3(-10.0f, -2.0f,  10.0f),
	glm::vec3(10.0f, -2.0f,  10.0f),
};

static const glm::vec3 cube_colors[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f),  // ��
	glm::vec3(0.0f, 1.0f, 0.0f),  // ��
	glm::vec3(0.0f, 0.0f, 1.0f),  // ��
	glm::vec3(1.0f, 1.0f, 0.0f),  // �C�G���[
	glm::vec3(0.0f, 1.0f, 1.0f),  // �V�A��
	glm::vec3(1.0f, 0.0f, 1.0f),  // �}�[���^
};

static const glm::vec2 texcoord[4] = {
	glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), //1
};

static const glm::vec3 background_colors[4] = {
	glm::vec3(0.025f, 0.025f, 0.05f),
	glm::vec3(0.025f, 0.025f, 0.05f),
	glm::vec3(0.15f,  0.2f,   0.25f),
	glm::vec3(0.15f,  0.2f,   0.25f), //tuning by Takahiro Iazuri
};

static const unsigned int cube_faces[12][3] = {
	{ 1, 6, 7 },{ 1, 7, 4 },
	{ 2, 5, 7 },{ 2, 7, 4 },
	{ 3, 5, 7 },{ 3, 7, 6 },
	{ 0, 1, 4 },{ 0, 4, 2 },
	{ 0, 1, 6 },{ 0, 6, 3 },
	{ 0, 2, 5 },{ 0, 5, 3 }
};

static const unsigned int background_faces[2][3] = {
	{ 1, 0, 2 },{ 1, 2, 3 }
};

// �o�b�t�@���Q�Ƃ���ԍ�
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

// �V�F�[�_���Q�Ƃ���ԍ�
GLuint programId;

// �����̂̉�]�p�x
static float theta = 0.0f;

// Arcball�R���g���[���̂��߂̕ϐ�
bool isDragging = false;

enum ArcballMode {
	ARCBALL_MODE_NONE = 0x00,
	ARCBALL_MODE_TRANSLATE = 0x01,
	ARCBALL_MODE_ROTATE = 0x02,
	ARCBALL_MODE_SCALE = 0x04
};

int arcballMode = ARCBALL_MODE_NONE;
glm::mat4 modelMat/*, viewMat, projMat*/;
glm::mat4 acRotMat, acTransMat, acScaleMat;
glm::vec3 gravity;

// �J�����̏�����
glm::mat4 projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

glm::mat4 viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // ���_�̈ʒu
	glm::vec3(0.0f, 0.0f, 0.0f),   // ���Ă����
	glm::vec3(0.0f, 1.0f, 0.0f));  // ���E�̏����

float acScale = 1.0f;
glm::ivec2 oldPos;
glm::ivec2 newPos;

class CubeManager {
public:
	CubeManager(
		const std::string shader_filename,
		const std::string obj_filename,
		const std::string texture_filename);//�R���X�g���N�^
	void c_initializeGL();
	void c_paintGL();

private:
	void c_initVAO();
	void initTexture();

public:
	// �V�F�[�_�t�@�C��
	const std::string c_VERT_SHADER_FILE;
	const std::string c_FRAG_SHADER_FILE;
	//���f���̃t�@�C��
	const std::string c_OBJECT_FILE;
	//�e�N�X�`���̃t�@�C��
	const std::string c_TEX_FILE;

	// �o�b�t�@���Q�Ƃ���ԍ�
	GLuint c_vaoId;

	// �C���f�b�N�X�o�b�t�@�̃T�C�Y (glDrawElements�Ŏg�p)
	size_t c_indexBufferSize;

	// �V�F�[�_���Q�Ƃ���ԍ�
	GLuint c_programId;

	//�e�N�X�`�����Q�Ƃ���ԍ�
	GLuint c_textureId;
};

class BackgroundManager {
public:
	BackgroundManager();//�R���X�g���N�^
	void b_initializeGL();
	void b_paintGL();

private:
	void b_initVAO();

public:
	// �V�F�[�_�t�@�C��
	const std::string b_VERT_SHADER_FILE;
	const std::string b_FRAG_SHADER_FILE;

	// �o�b�t�@���Q�Ƃ���ԍ�
	GLuint b_vaoId;

	// �V�F�[�_���Q�Ƃ���ԍ�
	GLuint b_programId;
};

//�I�u�W�F�N�g�p�R���X�g���N�^
CubeManager::CubeManager(const std::string shader_filename, const std::string obj_filename, const std::string texture_filename)
	//�V�F�[�_�t�@�C���̐ݒ�
	: c_VERT_SHADER_FILE(std::string(SHADER_DIRECTORY) + shader_filename + ".vert")
	, c_FRAG_SHADER_FILE(std::string(SHADER_DIRECTORY) + shader_filename + ".frag")
	//���f���̃t�@�C��
	, c_OBJECT_FILE(std::string(DATA_DIRECTORY) + obj_filename)
	//�e�N�X�`���̃t�@�C��
	, c_TEX_FILE(std::string(DATA_DIRECTORY) + texture_filename){
}

//�w�i�p�R���X�g���N�^
BackgroundManager::BackgroundManager()
//�V�F�[�_�t�@�C���̐ݒ�
	: b_VERT_SHADER_FILE(std::string(SHADER_DIRECTORY) + "background_render.vert")
	, b_FRAG_SHADER_FILE(std::string(SHADER_DIRECTORY) + "background_render.frag") {
}

// �L���[�u��VAO�̏�����
//void CubeManager::c_initVAO() {
//	// ���f���̃��[�h
//	tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materials;
//	std::string err;
//	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, c_OBJECT_FILE.c_str());
//	if (!err.empty()) {
//		std::cerr << "[WARNING] " << err << std::endl;
//	}
//
//	if (!success) {
//		std::cerr << "Failed to load OBJ file: " << c_OBJECT_FILE << std::endl;
//		exit(1);
//	}
//
//	// Vertex�z��̍쐬
//	std::vector<Vertex> vertices;
//	std::vector<unsigned int> indices;
//	for (int s = 0; s < shapes.size(); s++) {
//		const tinyobj::mesh_t &mesh = shapes[s].mesh;
//		for (int i = 0; i < mesh.indices.size(); i++) {
//			const tinyobj::index_t &index = mesh.indices[i];
//
//			glm::vec3 position, normal;
//
//			if (index.vertex_index >= 0) {
//				position = glm::vec3(attrib.vertices[index.vertex_index * 3 + 0],
//					attrib.vertices[index.vertex_index * 3 + 1],
//					attrib.vertices[index.vertex_index * 3 + 2]);
//			}
//
//			if (index.normal_index >= 0) {
//				normal = glm::vec3(attrib.normals[index.normal_index * 3 + 0],
//					attrib.normals[index.normal_index * 3 + 1],
//					attrib.normals[index.normal_index * 3 + 2]);
//			}
//
//			const unsigned int vertexIndex = vertices.size();
//			vertices.push_back(Vertex(position, normal));
//			indices.push_back(vertexIndex);
//		}
//	}
//
//	// VAO�̍쐬
//	glGenVertexArrays(1, &c_vaoId);
//	glBindVertexArray(c_vaoId);
//
//	// ���_�o�b�t�@�̍쐬
//	glGenBuffers(1, &vertexBufferId);
//	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
//
//	// ���_�o�b�t�@�̗L����
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
//
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
//
//	// ���_�ԍ��o�b�t�@�̍쐬
//	glGenBuffers(1, &indexBufferId);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
//		indices.data(), GL_STATIC_DRAW);
//
//	// ���_�o�b�t�@�̃T�C�Y��ϐ��ɓ���Ă���
//	c_indexBufferSize = indices.size();
//
//	// VAO��OFF�ɂ��Ă���
//	glBindVertexArray(0);
//}


void CubeManager::c_initVAO() {
	// Vertex�z��̍쐬
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int idx = 0;
	gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 3; j++) {
			Vertex v(cube_positions[cube_faces[i * 2 + 0][j]], cube_colors[i]);
			vertices.push_back(v);
			indices.push_back(idx++);
			gravity += v.position;
		}

		for (int j = 0; j < 3; j++) {
			Vertex v(cube_positions[cube_faces[i * 2 + 1][j]], cube_colors[i]);
			vertices.push_back(v);
			indices.push_back(idx++);
			gravity += v.position;
		}
	}
	gravity /= indices.size();

	// VAO�̍쐬
	glGenVertexArrays(1, &c_vaoId);
	glBindVertexArray(c_vaoId);

	// ���_�o�b�t�@�̍쐬
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// ���_�o�b�t�@�̗L����
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	// ���_�ԍ��o�b�t�@�̍쐬
	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
		indices.data(), GL_STATIC_DRAW);

	// VAO��OFF�ɂ��Ă���
	glBindVertexArray(0);
}

// �w�i��VAO�̏�����
void BackgroundManager::b_initVAO() {
	// Vertex�z��̍쐬
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int idx = 0;
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < 3; j++) {
			Vertex v(background_positions[background_faces[i * 2 + 0][j]], background_colors[background_faces[i * 2 + 0][j]]);
			vertices.push_back(v);
			indices.push_back(idx++);
		}

		for (int j = 0; j < 3; j++) {
			Vertex v(background_positions[background_faces[i * 2 + 1][j]], background_colors[background_faces[i * 2 + 1][j]]);
			vertices.push_back(v);
			indices.push_back(idx++);
		}
	}

	// VAO�̍쐬
	glGenVertexArrays(1, &b_vaoId);
	glBindVertexArray(b_vaoId);

	// ���_�o�b�t�@�̍쐬
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// ���_�o�b�t�@�̗L����
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	// ���_�ԍ��o�b�t�@�̍쐬
	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
		indices.data(), GL_STATIC_DRAW);

	// VAO��OFF�ɂ��Ă���
	glBindVertexArray(0);
}

GLuint compileShader(const std::string &filename, GLuint type) {
	// �V�F�[�_�̍쐬
	GLuint shaderId = glCreateShader(type);

	// �t�@�C���̓ǂݍ���
	std::ifstream reader;
	size_t codeSize;
	std::string code;

	// �t�@�C�����J��
	reader.open(filename.c_str(), std::ios::in);
	if (!reader.is_open()) {
		// �t�@�C�����J���Ȃ�������G���[���o���ďI��
		fprintf(stderr, "Failed to load a shader: %s\n", filename.c_str());
		exit(1);
	}

	// �t�@�C�������ׂēǂ�ŕϐ��Ɋi�[ (����)
	reader.seekg(0, std::ios::end);             // �t�@�C���ǂݎ��ʒu���I�[�Ɉړ� 
	codeSize = reader.tellg();                  // ���݂̉ӏ�(=�I�[)�̈ʒu���t�@�C���T�C�Y
	code.resize(codeSize);                      // �R�[�h���i�[����ϐ��̑傫����ݒ�
	reader.seekg(0);                            // �t�@�C���̓ǂݎ��ʒu��擪�Ɉړ�
	reader.read(&code[0], codeSize);            // �擪����t�@�C���T�C�Y����ǂ�ŃR�[�h�̕ϐ��Ɋi�[

												// �t�@�C�������
	reader.close();

	// �R�[�h�̃R���p�C��
	const char *codeChars = code.c_str();
	glShaderSource(shaderId, 1, &codeChars, NULL);
	glCompileShader(shaderId);

	// �R���p�C���̐��ۂ𔻒肷��
	GLint compileStatus;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		// �R���p�C�������s������G���[���b�Z�[�W�ƃ\�[�X�R�[�h��\�����ďI��
		fprintf(stderr, "Failed to compile a shader!\n");

		// �G���[���b�Z�[�W�̒������擾����
		GLint logLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			// �G���[���b�Z�[�W���擾����
			GLsizei length;
			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(shaderId, logLength, &length, &errMsg[0]);

			// �G���[���b�Z�[�W�ƃ\�[�X�R�[�h�̏o��
			fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
			fprintf(stderr, "%s\n", code.c_str());
		}
		exit(1);
	}

	return shaderId;
}

GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile) {
	// �V�F�[�_�̍쐬
	GLuint vertShaderId = compileShader(vShaderFile, GL_VERTEX_SHADER);
	GLuint fragShaderId = compileShader(fShaderFile, GL_FRAGMENT_SHADER);

	// �V�F�[�_�v���O�����̃����N
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertShaderId);
	glAttachShader(programId, fragShaderId);
	glLinkProgram(programId);

	// �����N�̐��ۂ𔻒肷��
	GLint linkState;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
	if (linkState == GL_FALSE) {
		// �����N�Ɏ��s������G���[���b�Z�[�W��\�����ďI��
		fprintf(stderr, "Failed to link shaders!\n");

		// �G���[���b�Z�[�W�̒������擾����
		GLint logLength;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			// �G���[���b�Z�[�W���擾����
			GLsizei length;
			std::string errMsg;
			errMsg.resize(logLength);
			glGetProgramInfoLog(programId, logLength, &length, &errMsg[0]);

			// �G���[���b�Z�[�W���o�͂���
			fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
		}
		exit(1);
	}

	// �V�F�[�_�𖳌����������ID��Ԃ�
	glUseProgram(0);
	return programId;
}

// �V�F�[�_�̏�����
void initShaders(GLuint& target_programId, const std::string &vShaderFile, const std::string &fShaderFile) {
	programId = buildShaderProgram(vShaderFile, fShaderFile);
	target_programId = programId;
}

void CubeManager::initTexture() {
	// �e�N�X�`���̐ݒ�
	int texWidth, texHeight, channels;
	unsigned char *bytes = stbi_load(c_TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
	if (!bytes) {
		fprintf(stderr, "Failed to load image file: %s\n", c_TEX_FILE.c_str());
		exit(1);
	}

	glGenTextures(1, &c_textureId);
	glBindTexture(GL_TEXTURE_2D, c_textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(bytes);
}

// OpenGL�̏������֐�
void CubeManager::c_initializeGL() {
	// �[�x�e�X�g�̗L����
	glEnable(GL_DEPTH_TEST);

	// �w�i�F�̐ݒ� (��)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// VAO�̏�����
	c_initVAO();

	// �V�F�[�_�̗p��
	initShaders(c_programId, c_VERT_SHADER_FILE, c_FRAG_SHADER_FILE);

	//// �J�����̏�����
	//projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

	//viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // ���_�̈ʒu
	//	glm::vec3(0.0f, 0.0f, 0.0f),   // ���Ă����
	//	glm::vec3(0.0f, 1.0f, 0.0f));  // ���E�̏����
}

// OpenGL�̏������֐�
void BackgroundManager::b_initializeGL() {
	// �[�x�e�X�g�̗L����
	//glEnable(GL_DEPTH_TEST);

	// �w�i�F�̐ݒ� (��)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// VAO�̏�����
	b_initVAO();

	// �V�F�[�_�̗p��
	initShaders(b_programId, b_VERT_SHADER_FILE, b_FRAG_SHADER_FILE);

	//// �J�����̏�����
	//projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

	//viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // ���_�̈ʒu
	//	glm::vec3(0.0f, 0.0f, 0.0f),   // ���Ă����
	//	glm::vec3(0.0f, 1.0f, 0.0f));  // ���E�̏����
}

// �L���[�u��OpenGL�̕`��֐�
void CubeManager::c_paintGL() {
	// �w�i�F�̕`��
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ���W�̕ϊ�
	//glm::mat4 projMat = glm::perspective(45.0f,
	//	(float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

	//glm::mat4 viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // ���_�̈ʒu
	//	glm::vec3(0.0f, 0.0f, 0.0f),   // ���Ă����
	//	glm::vec3(0.0f, 1.0f, 0.0f));  // ���E�̏����

	glm::mat4 modelMat = glm::rotate(glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 mvMat = viewMat * modelMat;
	glm::mat4 mvpMat = projMat * viewMat * modelMat;
	glm::mat4 normMat = glm::transpose(glm::inverse(mvMat));
	glm::mat4 lightMat = viewMat;

	// VAO�̗L����
	glBindVertexArray(c_vaoId);

	// �V�F�[�_�̗L����
	glUseProgram(c_programId);

	// Uniform�ϐ��̓]��
	GLuint uid;

	uid = glGetUniformLocation(programId, "u_mvpMat");
	glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));

	// �e�N�X�`���̗L�����ƃV�F�[�_�ւ̓]��
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, c_textureId);
	uid = glGetUniformLocation(programId, "u_texture");
	glUniform1i(uid, 0);

	// �O�p�`�̕`��
	glDrawElements(GL_TRIANGLES, c_indexBufferSize, GL_UNSIGNED_INT, 0);

	// VAO�̖�����
	glBindVertexArray(0);

	// �V�F�[�_�̖�����
	glUseProgram(0);

	// �e�N�X�`���̖�����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


//void CubeManager::c_paintGL() {
//	// �V�F�[�_�̗L����
//	glUseProgram(c_programId);
//
//	// VAO�̗L����
//	glBindVertexArray(c_vaoId);
//
//	// 1�ڂ̗����̂�`��
//	glm::mat4 mvpMat = projMat * viewMat * modelMat * acTransMat * acRotMat * acScaleMat;
//
//	// Uniform�ϐ��̓]��
//	GLuint uid;
//	uid = glGetUniformLocation(c_programId, "u_mvpMat");
//	glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
//
//	glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
//
//	// VAO�̖�����
//	glBindVertexArray(0);
//
//	// �V�F�[�_�̖�����
//	glUseProgram(0);
//}

// �w�i��OpenGL�̕`��֐�
void BackgroundManager::b_paintGL() {
	// �V�F�[�_�̗L����
	glUseProgram(b_programId);

	// VAO�̗L����
	glBindVertexArray(b_vaoId);

	// 1�ڂ̗����̂�`��
	glm::mat4 mvpMat = projMat * viewMat * modelMat * acTransMat * acRotMat * acScaleMat;

	// Uniform�ϐ��̓]��
	GLuint uid;
	uid = glGetUniformLocation(b_programId, "u_mvpMat");
	glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// VAO�̖�����
	glBindVertexArray(0);

	// �V�F�[�_�̖�����
	glUseProgram(0);
}

void resizeGL(GLFWwindow *window, int width, int height) {
	// ���[�U�Ǘ��̃E�B���h�E�T�C�Y��ύX
	WIN_WIDTH = width;
	WIN_HEIGHT = height;

	// GLFW�Ǘ��̃E�B���h�E�T�C�Y��ύX
	glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

	// ���ۂ̃E�B���h�E�T�C�Y (�s�N�Z����) ���擾
	int renderBufferWidth, renderBufferHeight;
	glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

	// �r���[�|�[�g�ϊ��̍X�V
	glViewport(0, 0, renderBufferWidth, renderBufferHeight);

	// ���e�ϊ��s��̏�����
	projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
}

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

int main(int argc, char **argv) {
	// OpenGL������������
	if (glfwInit() == GL_FALSE) {
		fprintf(stderr, "Initialization failed!\n");
		return 1;
	}

	// OpenGL�̃o�[�W�����ݒ� (Mac�̏ꍇ�ɂ͕K���K�v)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Window�̍쐬
	GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
		NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Window creation failed!");
		glfwTerminate();
		return 1;
	}

	// OpenGL�̕`��Ώۂ�Window��ǉ�
	glfwMakeContextCurrent(window);

	// �}�E�X�̃C�x���g����������֐���o�^
	glfwSetMouseButtonCallback(window, mouseEvent);
	glfwSetCursorPosCallback(window, mouseMoveEvent);
	glfwSetScrollCallback(window, wheelEvent);

	// GLEW������������ (glfwMakeContextCurrent�̌�łȂ��Ƃ����Ȃ�)
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "GLEW initialization failed!\n");
		return 1;
	}

	// �E�B���h�E�̃��T�C�Y�������֐��̓o�^
	glfwSetWindowSizeCallback(window, resizeGL);

	CubeManager cube("object_render", "cube.obj", "target01.jpg"); //�L���[�u�̃V�F�[�_
	BackgroundManager background;

	// OpenGL��������
	cube.c_initializeGL();
	background.b_initializeGL();

	// ���C�����[�v
	while (glfwWindowShouldClose(window) == GL_FALSE) {
		//�w�i�F�̕`��
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// �`��
		background.b_paintGL();
		cube.c_paintGL();

		// �`��p�o�b�t�@�̐؂�ւ�
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}