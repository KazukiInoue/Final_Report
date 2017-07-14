#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm>
#include <deque>
#include <random>

#define GLFW_INCLUDE_GLU  // GLUライブラリを使用するのに必要
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "common.h"

static int WIN_WIDTH = 800;                       // ウィンドウの幅
static int WIN_HEIGHT = 600;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル

static const float PI = 4.0 * std::atan(1.0);
double px = 0, py = 0; //マウスが押された位置

// 上白糖用のファイル
static const std::string SUGAR_OBJFILE = std::string(DATA_DIRECTORY) + "cube.obj";
static const std::string SUGAR_TEXFILE[3] = {
	 std::string(DATA_DIRECTORY) + "sugar.jpg",
	 std::string(DATA_DIRECTORY) + "black.jpg",
	 std::string(DATA_DIRECTORY) + "salt.jpg"
};

// 蟻用のファイル
static const std::string ant_OBJFILE = std::string(DATA_DIRECTORY) + "ant.obj";

// 三角錐用のファイル
static const std::string cone_OBJFILE = std::string(DATA_DIRECTORY) + "cone.obj";
static const std::string cone_TEXFILE = std::string(DATA_DIRECTORY) + "yellow.jpg";

// ゴール用のファイル
static const std::string goal_OBJFILE = std::string(DATA_DIRECTORY) + "cone.obj";
static const std::string goal_TEXFILE = std::string(DATA_DIRECTORY) + "salt.jpg";

// 空用のファイル
static const std::string SKY_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string SKY_TEXFILE = std::string(DATA_DIRECTORY) + "kitchen.jpg";

// 背景の赤
static const std::string RED_BACK_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string RED_BACK_TEXFILE = std::string(DATA_DIRECTORY) + "red.jpg";

// 背景の砂糖
static const std::string SUGAR_BACK_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string SUGAR_BACK_TEXFILE[3] = {
	 std::string(DATA_DIRECTORY) + "white_background.jpg",
	 std::string(DATA_DIRECTORY) + "black_background.jpg",
	 std::string(DATA_DIRECTORY) + "sio_background.jpg"
};

// 床用のファイル
static const std::string LIVING_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string LIVING_TEXFILE = std::string(DATA_DIRECTORY) + "living.jpg";

// スタート画面用のファイル
static const std::string START_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string START_TEXFILE = std::string(DATA_DIRECTORY) + "start.jpg";

// クリア画面用のファイル
static const std::string CLEAR_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string CLEAR_TEXFILE = std::string(DATA_DIRECTORY) + "clear.jpg";

// ゲームオーバー画面用のファイル
static const std::string OVER_OBJFILE = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string OVER_TEXFILE[2] = {
	 std::string(DATA_DIRECTORY) + "gameover_white.jpg",
	 std::string(DATA_DIRECTORY) + "gameover_black.jpg"
};

static const std::string RENDER_SHADER = std::string(SHADER_DIRECTORY) + "render";
static const std::string TEXTURE_SHADER = std::string(SHADER_DIRECTORY) + "texture";
static const std::string LIVING_SHADER = std::string(SHADER_DIRECTORY) + "living";
static const std::string BACKSUGAR_SHADER = std::string(SHADER_DIRECTORY) + "back_sugar";

static const glm::vec3 lightPos = glm::vec3(0.0f, 50.0f, 0.0f);

struct Vertex {
	Vertex()
		: position(0.0f, 0.0f, 0.0f)
		, normal(0.0f, 0.0f, 0.0f)
		, texcoord(0.0f, 0.0f) {
	}

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

struct Camera {
	glm::mat4 viewMat;
	glm::mat4 projMat;
};

Camera camera;

struct RenderObject {
	GLuint programId;
	GLuint vaoId;
	GLuint vboId;
	GLuint iboId;
	GLuint textureId;
	int bufferSize;

	glm::vec3 gravity;
	glm::vec3 initgravity;
	glm::vec3 object_origin;

	glm::vec3 x_min_vec;
	glm::vec3 y_min_vec;
	glm::vec3 y_max_vec;

	glm::mat4 modelMat;
	glm::mat4 TransMat;
	glm::mat4 RotMat;
	glm::mat4 ScaleMat;

	glm::vec3 ambiColor;
	glm::vec3 diffColor;
	glm::vec3 specColor;
	float shininess;

	void initialize() {
		programId = 0u;
		vaoId = 0u;
		vboId = 0u;
		iboId = 0u;
		textureId = 0u;
		bufferSize = 0;

		ambiColor = glm::vec3(0.0f, 0.0f, 0.0f);
		diffColor = glm::vec3(1.0f, 1.0f, 1.0f);
		specColor = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	void buildShader(const std::string &basename) {
		const std::string vertShaderFile = basename + ".vert";
		const std::string fragShaderFile = basename + ".frag";

		// シェーダの用意
		GLuint vertShaderId = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		// ファイルの読み込み (Vertex shader)
		std::ifstream vertFileInput(vertShaderFile.c_str(), std::ios::in);
		if (!vertFileInput.is_open()) {
			fprintf(stderr, "Failed to load vertex shader: %s\n", vertShaderFile.c_str());
			exit(1);
		}
		std::istreambuf_iterator<char> vertDataBegin(vertFileInput);
		std::istreambuf_iterator<char> vertDataEnd;
		const std::string vertFileData(vertDataBegin, vertDataEnd);
		const char *vertShaderCode = vertFileData.c_str();

		// ファイルの読み込み (Fragment shader)
		std::ifstream fragFileInput(fragShaderFile.c_str(), std::ios::in);
		if (!fragFileInput.is_open()) {
			fprintf(stderr, "Failed to load fragment shader: %s\n", fragShaderFile.c_str());
			exit(1);
		}
		std::istreambuf_iterator<char> fragDataBegin(fragFileInput);
		std::istreambuf_iterator<char> fragDataEnd;
		const std::string fragFileData(fragDataBegin, fragDataEnd);
		const char *fragShaderCode = fragFileData.c_str();

		// シェーダのコンパイル
		GLint compileStatus;
		glShaderSource(vertShaderId, 1, &vertShaderCode, NULL);
		glCompileShader(vertShaderId);
		glGetShaderiv(vertShaderId, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE) {
			fprintf(stderr, "Failed to compile vertex shader!\n");

			GLint logLength;
			glGetShaderiv(vertShaderId, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 0) {
				GLsizei length;
				char *errmsg = new char[logLength + 1];
				glGetShaderInfoLog(vertShaderId, logLength, &length, errmsg);

				std::cerr << errmsg << std::endl;
				fprintf(stderr, "%s", vertShaderCode);

				delete[] errmsg;
			}
		}

		glShaderSource(fragShaderId, 1, &fragShaderCode, NULL);
		glCompileShader(fragShaderId);
		glGetShaderiv(fragShaderId, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE) {
			fprintf(stderr, "Failed to compile fragment shader!\n");

			GLint logLength;
			glGetShaderiv(fragShaderId, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 0) {
				GLsizei length;
				char *errmsg = new char[logLength + 1];
				glGetShaderInfoLog(fragShaderId, logLength, &length, errmsg);

				std::cerr << errmsg << std::endl;
				fprintf(stderr, "%s", vertShaderCode);

				delete[] errmsg;
			}
		}

		// シェーダプログラムの用意
		programId = glCreateProgram();
		glAttachShader(programId, vertShaderId);
		glAttachShader(programId, fragShaderId);

		GLint linkState;
		glLinkProgram(programId);
		glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
		if (linkState == GL_FALSE) {
			fprintf(stderr, "Failed to link shaders!\n");

			GLint logLength;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 0) {
				GLsizei length;
				char *errmsg = new char[logLength + 1];
				glGetProgramInfoLog(programId, logLength, &length, errmsg);

				std::cerr << errmsg << std::endl;
				delete[] errmsg;
			}

			exit(1);
		}
	}

	void loadOBJ(const std::string &filename) {
		// Load OBJ file.
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
		if (!err.empty()) {
			std::cerr << "[WARNING] " << err << std::endl;
		}

		if (!success) {
			std::cerr << "Failed to load OBJ file: " << filename << std::endl;
			exit(1);
		}

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		initgravity = glm::vec3(0.0f, 0.0f, 0.0f);
		for (int s = 0; s < shapes.size(); s++) {
			const tinyobj::shape_t &shape = shapes[s];
			for (int i = 0; i < shape.mesh.indices.size(); i++) {
				const tinyobj::index_t &index = shapes[s].mesh.indices[i];


				Vertex vertex;
				if (index.vertex_index >= 0) {
					vertex.position = glm::vec3(
						attrib.vertices[index.vertex_index * 3 + 0],
						attrib.vertices[index.vertex_index * 3 + 1],
						attrib.vertices[index.vertex_index * 3 + 2]
					);

					initgravity += vertex.position;

					if (vertex.position.x < x_min_vec.x) { x_min_vec = vertex.position; }
					if (vertex.position.y < y_min_vec.y) { y_min_vec = vertex.position; }
					if (vertex.position.y > y_max_vec.y) { y_max_vec = vertex.position; }

				}

				if (index.normal_index >= 0) {
					vertex.normal = glm::vec3(
						attrib.normals[index.normal_index * 3 + 0],
						attrib.normals[index.normal_index * 3 + 1],
						attrib.normals[index.normal_index * 3 + 2]
					);
				}

				if (index.texcoord_index >= 0) {
					vertex.texcoord = glm::vec2(
						attrib.texcoords[index.texcoord_index * 2 + 0],
						1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]
					);
				}

				indices.push_back(vertices.size());
				vertices.push_back(vertex);
			}
		}
		initgravity /= indices.size();

		// Prepare VAO.
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);

		glGenBuffers(1, &vboId);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
			vertices.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

		glGenBuffers(1, &iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
			indices.data(), GL_STATIC_DRAW);
		bufferSize = indices.size();

		glBindVertexArray(0);
	}

	void loadTexture(const std::string &filename) {
		int texWidth, texHeight, channels;
		unsigned char *bytes = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
		if (!bytes) {
			fprintf(stderr, "Failed to load image file: %s\n", filename.c_str());
			exit(1);
		}

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(bytes);
	}

	void draw(const Camera &camera) {
		glUseProgram(programId);

		GLuint location;
		location = glGetUniformLocation(programId, "u_ambColor");
		glUniform3fv(location, 1, glm::value_ptr(ambiColor));
		location = glGetUniformLocation(programId, "u_diffColor");
		glUniform3fv(location, 1, glm::value_ptr(diffColor));
		location = glGetUniformLocation(programId, "u_specColor");
		glUniform3fv(location, 1, glm::value_ptr(specColor));
		location = glGetUniformLocation(programId, "u_shininess");
		glUniform1f(location, shininess);

		glm::mat4 mvMat, mvpMat, normMat;
		mvMat = camera.viewMat * modelMat;
		mvpMat = camera.projMat * mvMat * TransMat * RotMat * ScaleMat;
		normMat = glm::transpose(glm::inverse(mvMat));

		// オブジェクトの重心を移動
		gravity = glm::vec3(modelMat * TransMat * RotMat * ScaleMat * glm::vec4(initgravity, 1.0f));

		location = glGetUniformLocation(programId, "u_lightPos");
		glUniform3fv(location, 1, glm::value_ptr(lightPos));
		location = glGetUniformLocation(programId, "u_lightMat");
		glUniformMatrix4fv(location, 1, false, glm::value_ptr(camera.viewMat));
		location = glGetUniformLocation(programId, "u_mvMat");
		glUniformMatrix4fv(location, 1, false, glm::value_ptr(mvMat));
		location = glGetUniformLocation(programId, "u_mvpMat");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mvpMat));
		location = glGetUniformLocation(programId, "u_normMat");
		glUniformMatrix4fv(location, 1, false, glm::value_ptr(normMat));
		location = glGetUniformLocation(programId, "u_modelMat");
		glUniformMatrix4fv(location, 1, false, glm::value_ptr(modelMat));

		if (textureId != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureId);
			location = glGetUniformLocation(programId, "u_isTextured");
			glUniform1i(location, 1);
			location = glGetUniformLocation(programId, "u_texture");
			glUniform1i(location, 0);
		}
		else {
			location = glGetUniformLocation(programId, "u_isTextured");
			glUniform1i(location, 0);
		}

		glBindVertexArray(vaoId);
		glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(0);
	}
};

RenderObject sugar[3];
RenderObject ant[3];
RenderObject cone[3];
RenderObject goal[9];
RenderObject background;
RenderObject red_back[3];
RenderObject sugar_back[3];
RenderObject living;
RenderObject startDisp;
RenderObject clearDisp;
RenderObject overDisp[2];


static const int sizeof_sugar = sizeof sugar / sizeof sugar[0];
static const int sizeof_ant = sizeof ant / sizeof ant[0];
static const int sizeof_goal = sizeof goal / sizeof goal[0];
static const int sizeof_red_back = sizeof red_back / sizeof red_back[0];


glm::vec3 prevSpeed[3]; //敵の前の移動方向ベクトル
glm::vec3 nextSpeed[3]; //敵の現在の移動方向ベクトル

static const float SPEED = 0.03f;
static const float SUGAR_SCALE = 4.0f;
static const float LIVING_SCALE = 50.0f;
static const float ANT_SCALE = 2000.0f;

bool isRotating[3] = {};

static const float theta_const = 2.0f * PI / 1000.0f;
float theta_collision[3] = {};
glm::vec3 rotAxis[3];
int rotCount[3] = { 1,1,1 };

glm::vec3 initconeHead[3];
glm::vec3 coneHead[3];
glm::vec3 coneBus[2]; // coneの開き角を計算するには、1つのconeを見れば十分	
float coneAngle;

enum {
	GAME_MODE_START,
	GAME_MODE_WHITE,
	GAME_MODE_BLACK,
	GAME_MODE_SIO,
	GAME_MODE_CLEAR,
	GAME_MODE_OVER_WHITE,
	GAME_MODE_OVER_BLACK,
};

int gameMode = GAME_MODE_START;

void gameInit() {
	// 砂糖の位置の初期化
	glm::vec3 trans_sugar = glm::vec3(0.0f, -11.0f, 40.0f);
	for (int i = 0; i < sizeof_sugar; i++) {
		sugar[i].modelMat = glm::mat4();
		sugar[i].ScaleMat = glm::scale(glm::mat4(), glm::vec3(SUGAR_SCALE, SUGAR_SCALE, SUGAR_SCALE));
		sugar[i].TransMat = glm::translate(glm::mat4(), trans_sugar);
		sugar[i].RotMat = glm::mat4();
		sugar[i].gravity = glm::vec3(sugar[i].TransMat * sugar[i].RotMat * sugar[i].ScaleMat * glm::vec4(sugar[i].initgravity, 1.0f));
	}

	// 床の初期化
	living.RotMat = glm::rotate(PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	living.ScaleMat = glm::scale(glm::mat4(), glm::vec3(LIVING_SCALE, LIVING_SCALE, 1.0f));
	living.TransMat = glm::translate(glm::mat4(), glm::vec3(0.0f, -13.0f, 0.0f));

	// 背景の砂糖の初期化
	glm::vec3 trans_back[3] = {
		glm::vec3(-0.3f, 0.8f, 0.0f),
		glm::vec3(0.3f, 0.8f, 0.0f),
		glm::vec3(-0.8f, 0.8f, 0.0f)
	};

	for (int i = 0; i < sizeof_red_back; i++) {
		red_back[i].modelMat = glm::translate(glm::mat4(), trans_back[i]) * glm::scale(glm::mat4(), glm::vec3(0.15f, 0.15f, 0.15f));
		sugar_back[i].modelMat = glm::translate(glm::mat4(), trans_back[i]) * glm::scale(glm::mat4(), glm::vec3(0.1f, 0.1f, 0.1f));
	}

	// ゴールの配置
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			glm::vec3 trans_goal = glm::vec3(-35.0f - 5 * i, -13.0f, -35.0f - 5 * j);
			goal[3 * i + j].ScaleMat = glm::scale(glm::mat4(), glm::vec3(1.0f, 5.0f, 5.0f));
			goal[3 * i + j].RotMat = glm::rotate(-PI / 2, glm::vec3(0.0f, 0.0f, 1.0f));
			goal[3 * i + j].TransMat = glm::translate(glm::mat4(), trans_goal);
		}
	}

	// 蟻の配置
	glm::vec3 trans_initant[3] = {
		glm::vec3(40.0f, -3.0f, -15.0f),
		glm::vec3(0.0f, -3.0f, -15.0f - 25.0f),
		glm::vec3(-40.0f, -3.0f, -15.0f)
	};

	for (int i = 0; i < sizeof_ant; i++) {
		ant[i].modelMat = glm::mat4();
		ant[i].ScaleMat = glm::scale(glm::mat4(), glm::vec3(ANT_SCALE, ANT_SCALE, ANT_SCALE));
		ant[i].RotMat = glm::mat4();
		ant[i].TransMat = glm::translate(glm::mat4(), trans_initant[i]);
		ant[i].gravity = glm::vec3(ant[i].TransMat * ant[i].RotMat * ant[i].ScaleMat * glm::vec4(ant[i].initgravity, 1.0f));
	}

	// 視野の配置
	glm::vec3 trans_initcone[3] = {
		glm::vec3(40.0f, -3.0f, 28.0f),
		glm::vec3(0.0f, -3.0f, 28.0f - 25.0f),
		glm::vec3(-40.0f, -3.0f, 28.0f)
	};
	for (int i = 0; i < sizeof_ant; i++) {
		cone[i].modelMat = glm::mat4();
		cone[i].ScaleMat = glm::scale(glm::mat4(), glm::vec3(6.0f, 12.0f, 12.0f));
		cone[i].RotMat = glm::rotate(glm::mat4(), -PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
		cone[i].TransMat = glm::translate(glm::mat4(), trans_initcone[i]);
	}

	// 視野の開き角の設定
	for (int i = 0; i < sizeof_ant; i++) {
		initconeHead[i] = cone[i].x_min_vec;
	}

	coneHead[0] = glm::vec3(cone[0].modelMat * cone[0].TransMat * cone[0].RotMat * cone[0].ScaleMat * glm::vec4(initconeHead[0], 1.0f));
	coneBus[0] = glm::vec3(glm::translate(glm::mat4(), glm::vec3(SUGAR_SCALE, 0.0f, 0.0f)) * cone[0].modelMat * cone[0].TransMat * cone[0].RotMat * cone[0].ScaleMat * glm::vec4(cone[0].y_max_vec, 1.0f)); // SUGAR_SCALEだけ下駄をhかせる
	coneBus[1] = glm::vec3(glm::translate(glm::mat4(), glm::vec3(-SUGAR_SCALE, 0.0f, 0.0f)) * cone[0].modelMat * cone[0].TransMat * cone[0].RotMat * cone[0].ScaleMat * glm::vec4(cone[0].y_min_vec, 1.0f));	// SUGAR_SCALEだけ下駄をhかせる
	coneAngle = acos(dot(glm::normalize(coneBus[0] - coneHead[0]), glm::normalize(coneBus[1] - coneHead[0])));

	glm::vec3 initfacevec = glm::vec3(0.0f, 0.0f, 1.0f); // アリは初期状態でz方向を向いている
	srand((unsigned int)time(NULL));
	std::random_device rnd;     // 非決定的な乱数生成器を生成
	std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
	std::uniform_int_distribution<> rand(-100.0, 100.0);        // [-100, 100] 範囲の一様乱数

	for (int i = 0; i < sizeof_ant; i++) {
		prevSpeed[i] = initfacevec;
		nextSpeed[i] = glm::vec3((float)rand(mt), (float)rand(mt), (float)rand(mt)); // キャストは不可欠
		for (int j = 0; j < 3; j++) {
			nextSpeed[i][j] *= SPEED / length(nextSpeed[i]);
		}
		//nextSpeed[i] = glm::vec3(SPEED / 1.4, 0.0f, SPEED / 1.4);
		//nextSpeed[i] = glm::vec3(0.0f, 0.0f, SPEED);

		glm::vec3 u = glm::normalize(nextSpeed[i]);
		glm::vec3 v = glm::normalize(prevSpeed[i]);

		// ワールド座標系における回転量
		float inittheta = glm::acos(std::max(-1.0f, std::min(dot(u, v), 1.0f)));

		// ワールド座標系における回転軸
		glm::vec3 initrotAxis = glm::cross(v, u);

		// ant[i]の重心を中心に回転
		ant[i].modelMat =
			glm::translate(glm::mat4(), ant[i].gravity)
			* glm::rotate(glm::mat4(), inittheta, initrotAxis)
			* glm::translate(glm::mat4(), -ant[i].gravity)
			* ant[i].modelMat;
		cone[i].modelMat =
			glm::translate(glm::mat4(), ant[i].gravity)
			* glm::rotate(glm::mat4(), inittheta, initrotAxis)
			* glm::translate(glm::mat4(), -ant[i].gravity)
			* cone[i].modelMat;

		// 初期化
		isRotating[i] = false;
		theta_collision[i] = 0.0f;
		rotCount[i] = 1;
	}
}

void initializeGL() {
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < sizeof_sugar; i++) {
		sugar[i].initialize();
		sugar[i].loadOBJ(SUGAR_OBJFILE);
		sugar[i].buildShader(RENDER_SHADER);
		sugar[i].loadTexture(SUGAR_TEXFILE[i]);
	}

	for (int i = 0; i < sizeof_ant; i++) {
		ant[i].initialize();
		ant[i].loadOBJ(ant_OBJFILE);
		ant[i].buildShader(RENDER_SHADER);
		ant[i].diffColor = glm::vec3(0.1f, 0.1f, 0.1f);
		ant[i].specColor = glm::vec3(0.2f, 0.2f, 0.2f);
		ant[i].ambiColor = glm::vec3(0.1f, 0.0f, 0.0f);

		cone[i].initialize();
		cone[i].loadOBJ(cone_OBJFILE);
		cone[i].buildShader(RENDER_SHADER);
		cone[i].loadTexture(cone_TEXFILE);
	}

	for (int i = 0; i < sizeof_goal; i++) {
		goal[i].initialize();
		goal[i].loadOBJ(goal_OBJFILE);
		goal[i].buildShader(RENDER_SHADER);
		goal[i].loadTexture(goal_TEXFILE);
	}

	background.initialize();
	background.loadOBJ(SKY_OBJFILE);
	background.buildShader(TEXTURE_SHADER);
	background.loadTexture(SKY_TEXFILE);

	for (int i = 0; i < sizeof_red_back; i++) {
		red_back[i].initialize();
		red_back[i].loadOBJ(RED_BACK_OBJFILE);
		red_back[i].buildShader(BACKSUGAR_SHADER);
		red_back[i].loadTexture(RED_BACK_TEXFILE);

		sugar_back[i].initialize();
		sugar_back[i].loadOBJ(SUGAR_BACK_OBJFILE);
		sugar_back[i].buildShader(BACKSUGAR_SHADER);
		sugar_back[i].loadTexture(SUGAR_BACK_TEXFILE[i]);
	}

	living.initialize();
	living.loadOBJ(LIVING_OBJFILE);
	living.buildShader(LIVING_SHADER);
	living.loadTexture(LIVING_TEXFILE);

	startDisp.initialize();
	startDisp.loadOBJ(START_OBJFILE);
	startDisp.buildShader(TEXTURE_SHADER);
	startDisp.loadTexture(START_TEXFILE);

	clearDisp.initialize();
	clearDisp.loadOBJ(CLEAR_OBJFILE);
	clearDisp.buildShader(TEXTURE_SHADER);
	clearDisp.loadTexture(CLEAR_TEXFILE);

	for (int i = 0; i < 2; i++) {
		overDisp[i].initialize();
		overDisp[i].loadOBJ(OVER_OBJFILE);
		overDisp[i].buildShader(TEXTURE_SHADER);
		overDisp[i].loadTexture(OVER_TEXFILE[i]);
	}

	camera.projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
	camera.viewMat = glm::lookAt(
		glm::vec3(0.0f, 70.0f, 80.0f),
		glm::vec3(0.0f, -13.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	gameInit();
}

void paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	background.draw(camera);
	for (int i = 0; i < 2; i++) {
		red_back[i].draw(camera);
		sugar_back[i].draw(camera);
	}
	glEnable(GL_DEPTH_TEST);
	living.draw(camera);
	for (int i = 0; i < 9; i++) {
		goal[i].draw(camera);
	}

	switch (gameMode) {
	case GAME_MODE_START:
	{
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		startDisp.draw(camera);
		glEnable(GL_DEPTH_TEST);
	}
	break;

	case GAME_MODE_WHITE:
	{
		sugar[0].draw(camera);
		for (int i = 0; i < sizeof_ant; i++) {
			ant[i].draw(camera);
			cone[i].draw(camera);
		}
	}
	break;

	case GAME_MODE_BLACK:
	{
		sugar[1].draw(camera);
		for (int i = 0; i < sizeof_ant; i++) {
			ant[i].draw(camera);
			cone[i].draw(camera);
		}
	}
	break;

	case GAME_MODE_SIO:
	{
		glDisable(GL_DEPTH_TEST);
		red_back[2].draw(camera);
		sugar_back[2].draw(camera);
		glEnable(GL_DEPTH_TEST);
		sugar[2].draw(camera);
		for (int i = 0; i < sizeof_ant; i++) {
			ant[i].draw(camera);
			cone[i].draw(camera);
		}
	}
	break;

	case GAME_MODE_CLEAR:
	{
		glDisable(GL_DEPTH_TEST);
		clearDisp.draw(camera);
		glEnable(GL_DEPTH_TEST);
	}
	break;

	case GAME_MODE_OVER_WHITE:
	{
		glDisable(GL_DEPTH_TEST);
		overDisp[0].draw(camera);
		glEnable(GL_DEPTH_TEST);
	}
	break;

	case GAME_MODE_OVER_BLACK:
	{
		glDisable(GL_DEPTH_TEST);
		overDisp[1].draw(camera);
		glEnable(GL_DEPTH_TEST);
	}
	break;
	}
}

void resizeGL(GLFWwindow *window, int width, int height) {
	// ユーザ管理のウィンドウサイズを変更
	WIN_WIDTH = width;
	WIN_HEIGHT = height;

	// GLFW管理のウィンドウサイズを変更
	glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

	// 実際のウィンドウサイズ (ピクセル数) を取得
	int renderBufferWidth, renderBufferHeight;
	glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

	// ビューポート変換の更新
	glViewport(0, 0, renderBufferWidth, renderBufferHeight);

	// カメラ行列の更新
	float aspect = (float)WIN_WIDTH / (float)WIN_HEIGHT;
	camera.projMat = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);
}

void update() {
	if (gameMode == GAME_MODE_WHITE || gameMode == GAME_MODE_BLACK || gameMode == GAME_MODE_SIO) {

		// 敵が外に行かないような処理
		for (int i = 0; i < sizeof_ant; i++) {
			if (!isRotating[i]) {
				if (ant[i].gravity.x > (1 - 0.66 * i) * 0.9 * LIVING_SCALE || ant[i].gravity.x < (1 - 0.66 * (i + 1)) * 0.9 * LIVING_SCALE) {
					prevSpeed[i] = nextSpeed[i];
					nextSpeed[i].x *= -1;
					isRotating[i] = true;
				}
				else if (ant[i].gravity.y > 10.0 || ant[i].gravity.y < -10.0) {
					prevSpeed[i] = nextSpeed[i];
					nextSpeed[i].y *= -1;
					isRotating[i] = true;
				}
				else if (ant[i].gravity.z > 0.9*LIVING_SCALE || ant[i].gravity.z < -0.9*LIVING_SCALE) {
					prevSpeed[i] = nextSpeed[i];
					nextSpeed[i].z *= -1;
					isRotating[i] = true;
				}

				// 回転軸、回転角の決定
				if (isRotating[i]) {
					const glm::vec3 u = glm::normalize(nextSpeed[i]);
					const glm::vec3 v = glm::normalize(prevSpeed[i]);

					// ワールド座標系における回転量
					theta_collision[i] = glm::acos(std::max(-1.0f, std::min(dot(u, v), 1.0f)));

					// ワールド座標系における回転軸
					rotAxis[i] = glm::cross(v, u);
				}
			}

			// 上のif文でisRotating[i]=trueになったと同時に、このif文に入ってほしいので、elseではダメ
			if (isRotating[i]) {
				theta_collision[i] -= theta_const;
				if (theta_collision[i] > theta_const) {
					// ant[i]の重心を中心に回転
					ant[i].modelMat =
						glm::translate(glm::mat4(), ant[i].gravity)
						* glm::rotate(glm::mat4(), theta_const, rotAxis[i])
						* glm::translate(glm::mat4(), -ant[i].gravity)
						* ant[i].modelMat;
					cone[i].modelMat =
						glm::translate(glm::mat4(), ant[i].gravity)
						* glm::rotate(glm::mat4(), theta_const, rotAxis[i])
						* glm::translate(glm::mat4(), -ant[i].gravity)
						* cone[i].modelMat;
				}
				else if (theta_collision[i] < theta_const && theta_collision[i] > 0) {
					// ant[i]の重心を中心に残り分回転
					ant[i].modelMat =
						glm::translate(glm::mat4(), ant[i].gravity)
						* glm::rotate(glm::mat4(), theta_collision[i], rotAxis[i])
						* glm::translate(glm::mat4(), -ant[i].gravity)
						* ant[i].modelMat;
					cone[i].modelMat =
						glm::translate(glm::mat4(), ant[i].gravity)
						* glm::rotate(glm::mat4(), theta_collision[i], rotAxis[i])
						* glm::translate(glm::mat4(), -ant[i].gravity)
						* cone[i].modelMat;

					ant[i].modelMat = glm::translate(glm::mat4(), nextSpeed[i] + nextSpeed[i]) * ant[i].modelMat;
					cone[i].modelMat = glm::translate(glm::mat4(), nextSpeed[i] + nextSpeed[i]) * cone[i].modelMat;

					isRotating[i] = false;
					theta_collision[i] = 0.0f;
					rotCount[i] = 1;
				}
			}
			else {
				// 敵の移動
				ant[i].modelMat = glm::translate(glm::mat4(), nextSpeed[i]) * ant[i].modelMat;
				cone[i].modelMat = glm::translate(glm::mat4(), nextSpeed[i]) * cone[i].modelMat;
			}

			coneHead[i] = glm::vec3(cone[i].modelMat * cone[i].TransMat * cone[i].RotMat * cone[i].ScaleMat * glm::vec4(initconeHead[i], 1.0f));
		}
	}

	if (px > 241 && px < 317 && py > 29 && py < 86) {
		gameMode = GAME_MODE_WHITE;
		px = 0; py = 0;
	}
	if (px > 480 && px < 555 && py > 28 && py < 86) {
		gameMode = GAME_MODE_BLACK;
		px = 0; py = 0;
	}
	if (px > 40 && px < 120 && py > 30 && py < 90) {
		gameMode = GAME_MODE_SIO;
		px = 0; py = 0;
	}

	// 塩に近づくと蟻は固まる
	if (gameMode == GAME_MODE_SIO) {
		for (int i = 0; i < sizeof_ant; i++) {
			if (length(ant[i].gravity - sugar[2].gravity) < 30.0f) {
				ant[i].modelMat = inverse(glm::translate(glm::mat4(), nextSpeed[i])) * ant[i].modelMat;
				cone[i].modelMat = inverse(glm::translate(glm::mat4(), nextSpeed[i])) * cone[i].modelMat;
			}
		}
	}

	if ((gameMode == GAME_MODE_WHITE && length(glm::vec3(-LIVING_SCALE, 0, -LIVING_SCALE) - sugar[0].gravity) < 20.0f)
		|| (gameMode == GAME_MODE_BLACK && length(glm::vec3(-LIVING_SCALE, 0, -LIVING_SCALE) - sugar[1].gravity) < 20.0f)
		|| (gameMode == GAME_MODE_SIO && length(glm::vec3(-LIVING_SCALE, 0, -LIVING_SCALE) - sugar[2].gravity) < 20.0f)) {
		gameMode = GAME_MODE_CLEAR;
	}

	if (gameMode == GAME_MODE_WHITE
		&& ((length(sugar[0].gravity - coneHead[0]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[0]), glm::normalize(sugar[0].gravity - coneHead[0]))) < coneAngle)
			|| (length(sugar[0].gravity - coneHead[1]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[1]), glm::normalize(sugar[0].gravity - coneHead[1]))) < coneAngle)
			|| (length(sugar[0].gravity - coneHead[2]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[2]), glm::normalize(sugar[0].gravity - coneHead[2]))) < coneAngle)
			)) {
		gameMode = GAME_MODE_OVER_WHITE;
	}

	if (gameMode == GAME_MODE_BLACK
		&& ((length(sugar[1].gravity - coneHead[0]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[0]), glm::normalize(sugar[1].gravity - coneHead[0]))) < coneAngle)
			|| (length(sugar[1].gravity - coneHead[1]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[1]), glm::normalize(sugar[1].gravity - coneHead[1]))) < coneAngle)
			|| (length(sugar[1].gravity - coneHead[2]) < 40.0f && 2 * acos(dot(glm::normalize(nextSpeed[2]), glm::normalize(sugar[1].gravity - coneHead[2]))) < coneAngle)
			)) {
		gameMode = GAME_MODE_OVER_BLACK;
	}
}

void keyboard(GLFWwindow *window) {

	if (gameMode == GAME_MODE_WHITE || gameMode == GAME_MODE_BLACK || gameMode == GAME_MODE_SIO) {
		int state;
		glm::vec3 trans_sugar;
		// 上
		state = glfwGetKey(window, GLFW_KEY_UP);
		if ((state == GLFW_PRESS || state == GLFW_REPEAT)
			&& (sugar[0].gravity.z - SUGAR_SCALE / 2 > -LIVING_SCALE
				&& sugar[1].gravity.z - SUGAR_SCALE / 2 > -LIVING_SCALE
				&& sugar[2].gravity.z - SUGAR_SCALE / 2 > -LIVING_SCALE)) {
			trans_sugar = glm::vec3(0.0f, 0.0f, -0.1f);
		}

		// 下
		state = glfwGetKey(window, GLFW_KEY_DOWN);
		if ((state == GLFW_PRESS || state == GLFW_REPEAT)
			&& (sugar[0].gravity.z + SUGAR_SCALE / 2 < LIVING_SCALE
				&& sugar[1].gravity.z + SUGAR_SCALE / 2 < LIVING_SCALE
				&& sugar[2].gravity.z + SUGAR_SCALE / 2 < LIVING_SCALE)) {
			trans_sugar = glm::vec3(0.0f, 0.0f, 0.1f);
		}

		// 左
		state = glfwGetKey(window, GLFW_KEY_LEFT);
		if ((state == GLFW_PRESS || state == GLFW_REPEAT)
			&& (sugar[0].gravity.x - SUGAR_SCALE / 2 > -LIVING_SCALE
				&& sugar[1].gravity.x - SUGAR_SCALE / 2 > -LIVING_SCALE
				&& sugar[2].gravity.x - SUGAR_SCALE / 2 > -LIVING_SCALE)) {
			trans_sugar = glm::vec3(-0.1f, 0.0f, 0.0f);
		}

		// 右
		state = glfwGetKey(window, GLFW_KEY_RIGHT);
		if ((state == GLFW_PRESS || state == GLFW_REPEAT)
			&& (sugar[0].gravity.x + SUGAR_SCALE / 2 < LIVING_SCALE
				&& sugar[1].gravity.x + SUGAR_SCALE / 2 < LIVING_SCALE
				&& sugar[2].gravity.x + SUGAR_SCALE / 2 < LIVING_SCALE)) {
			trans_sugar = glm::vec3(0.1f, 0.0f, 0.0f);
		}

		for (int i = 0; i < sizeof_sugar; i++) {
			sugar[i].modelMat = glm::translate(glm::mat4(), trans_sugar) * sugar[i].modelMat;
			sugar[i].gravity = glm::vec3(sugar[i].modelMat * sugar[i].TransMat * sugar[i].RotMat * sugar[i].ScaleMat * glm::vec4(sugar[i].initgravity, 1.0f));

		}
	}
}

void keyboardCallback(GLFWwindow *window, int key, int scanmode, int action, int mods) {
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		if (gameMode == GAME_MODE_START) {
			gameMode = GAME_MODE_WHITE;
		}
		else if (gameMode == GAME_MODE_CLEAR || gameMode == GAME_MODE_OVER_WHITE || gameMode == GAME_MODE_OVER_BLACK) {
			gameMode = GAME_MODE_START;
			gameInit();
		}
	}
}

void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
	// クリックされた位置を取得
	glfwGetCursorPos(window, &px, &py);
}

int main(int argc, char **argv) {
	// OpenGLを初期化する
	if (glfwInit() == GL_FALSE) {
		fprintf(stderr, "Initialization failed!\n");
		return 1;
	}

	// OpenGLのバージョン指定
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Windowの作成
	GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
		NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Window creation failed!");
		glfwTerminate();
		return 1;
	}

	// OpenGLの描画対象にWindowを追加
	glfwMakeContextCurrent(window);

	// GLEWを初期化する (glfwMakeContextnextの後でないといけない)
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "GLEW initialization failed!\n");
		return 1;
	}

	// ウィンドウのリサイズを扱う関数の登録
	glfwSetWindowSizeCallback(window, resizeGL);

	// OpenGLの描画対象にWindowを追加
	glfwMakeContextCurrent(window);

	// キーボードコールバック関数の登録
	glfwSetKeyCallback(window, keyboardCallback);

	// マウスのイベントを処理する関数を登録
	glfwSetMouseButtonCallback(window, mouseEvent);

	// OpenGLを初期化
	initializeGL();

	// メインループ
	while (glfwWindowShouldClose(window) == GL_FALSE) {

		// 描画
		paintGL();

		// アニメーション
		//update();

		// キーボード処理
		keyboard(window);

		// 描画用バッファの切り替え
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}