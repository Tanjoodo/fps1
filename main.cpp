#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/gl.h"

#include <iostream>
#include <iterator>
#include <string>
#include <fstream>
#include <streambuf>
#include <chrono>
#include <cstdint>

GLuint cube_vbo;
GLuint cube_vao;
GLuint cube_prog;
GLint cube_rot;
std::string vshader, fshader;
void draw();
void update();


std::uint64_t current_time;
struct point {
	static const int BUFF_SIZE = 3 * sizeof(float);
	static const int BUFF_LEN = BUFF_SIZE / sizeof(float);
	float points[3];
	point() { }
	point(const float x, const float y, const float z) {
		points[0] = x;
		points[1] = y;
		points[2] = z;
	}

	point(float points[]) {
		for (int i = 0; i < 3; ++i) {
			this->points[i] = points[i];
		}
	}

	void print() const {
		std::cout << points[0] << ' ';
		std::cout << points[1] << ' ';
		std::cout << points[2];
	}
};

struct triangle {
	static const int BUFF_SIZE = 3 * point::BUFF_SIZE;
	static const int BUFF_LEN = BUFF_SIZE / sizeof(float);
	float pointbuff[BUFF_LEN];
	triangle() { }
	triangle(const point p1, const point p2, const point p3) {
		for (int i = 0; i < 3; ++i) {
			pointbuff[i] = p1.points[i];
			pointbuff[i + 3] = p2.points[i];
			pointbuff[i + 6] = p3.points[i];
		}
	}
};

struct face {
	static const int BUFF_SIZE = 2 * triangle::BUFF_SIZE;
	static const int BUFF_LEN = BUFF_SIZE / sizeof(float);
	float pointbuff[BUFF_LEN];
	face() { }
	face(const point& p1, const point& p2, const point& p3, const point& p4) {
		triangle t1, t2;
		t1 = triangle(p1, p2, p3);
		t2 = triangle(p3, p4, p1);
		std::copy(std::begin(t1.pointbuff), std::end(t1.pointbuff), std::begin(pointbuff));
		std::copy(std::begin(t2.pointbuff), std::end(t2.pointbuff), std::begin(pointbuff) + triangle::BUFF_LEN);
	}
};

const int CUBE_SIZE = face::BUFF_SIZE * 6;
const int CUBE_LEN = face::BUFF_LEN * 6;

std::string read_file(std::string file_name) {
	std::ifstream f(file_name);
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return str;
}

bool compile_shader(std::string source, GLenum type,  GLuint& shader) {
	shader = glCreateShader(type);
	int len = source.length();
	const char* cstr = source.c_str();
	glShaderSource(shader, 1, (const GLchar**)&cstr, &len);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		std::cout << "Error compiling shader " << std::endl;
		std::cout << source << std::endl;
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetShaderInfoLog(shader, logSize, NULL, logMsg);
		std::cout << logMsg << std::endl;
		delete[] logMsg;
		return false;
	}

	return true;

}

int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(500, 500);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(4, 1);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutCreateWindow("Hello FPS");
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cout << "Error: " << glewGetErrorString(err) << '\n';
	}
	// load a cube into a buffer
	point cube_points[] = {
		point(-1, -1, -1), // 0 bottom left down
		point(1, -1, -1),  // 1 bottom right down
		point(1, -1, 1),   // 2 top right down
		point(-1, -1, 1),  // 3 top left down
		point(-1, 1, -1),  // 4 bottom left up
		point(1, 1, -1),   // 5 bottom right up
		point(1, 1, 1),    // 6 top right up
		point(-1, 1, 1)    // 7 top left up
	};

	face cube_faces[] = {
		face(cube_points[0], cube_points[1], cube_points[2], cube_points[3]), // 0 bottom face
		face(cube_points[0], cube_points[1], cube_points[5], cube_points[4]), // 1 front face
		face(cube_points[0], cube_points[3], cube_points[7], cube_points[4]), // 2 left face
		face(cube_points[6], cube_points[7], cube_points[4], cube_points[5]), // 3 top face
		face(cube_points[6], cube_points[5], cube_points[1], cube_points[2]), // 4 right face
		face(cube_points[6], cube_points[7], cube_points[3], cube_points[2])  // 5 back face
	};

	float cube_buffer[CUBE_LEN];
	for (int i = 0; i < 6; ++i) {
		std::copy(std::begin(cube_faces[i].pointbuff), std::end(cube_faces[i].pointbuff), std::begin(cube_buffer) + i * face::BUFF_LEN);
	}

	std::cout << "Cube len: " << CUBE_LEN << std::endl;
	for (int i = 0; i < CUBE_LEN; ++i ) {
		if (i % 3 == 0) {
			std::cout << std::endl;
		}
		std::cout << cube_buffer[i] << ' ';
	}

	std::cout << std::endl;

	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	glGenBuffers(1, &cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, CUBE_SIZE, cube_buffer, GL_STATIC_DRAW);

	vshader = read_file("vshader.glsl");
	fshader = read_file("fshader.glsl");

	cube_prog = glCreateProgram();
	GLuint v, f;

	if (!(compile_shader(vshader, GL_VERTEX_SHADER, v) && compile_shader(fshader, GL_FRAGMENT_SHADER, f))) {
		return 1;
	}

	glAttachShader(cube_prog, v);
	glAttachShader(cube_prog, f);

	glLinkProgram(cube_prog);
	GLint linked;
	glGetProgramiv(cube_prog, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv(cube_prog, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog( cube_prog, logSize, NULL, logMsg );
		std::cerr << logMsg << std::endl;
		delete [] logMsg;
		return 2;

	}

	GLuint loc = glGetAttribLocation(cube_prog, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glClearColor(1.,1.,1.,1.);
	glUseProgram(cube_prog);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	cube_rot = glGetUniformLocation(cube_prog, "rot");

	current_time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	glutDisplayFunc(draw);
	glutIdleFunc(update);
	glutMainLoop();
	return 0;
}

float angle = 0.0;
void draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1f(cube_rot, angle);
	glDrawArrays(GL_TRIANGLES, 0, CUBE_LEN);
	glFlush();
}

void update() {
	std::uint64_t now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	auto delta = now - current_time;
	angle += 0.1 * delta;
	if (angle > 360)
		angle = 0.0;
	current_time = now;
	glutPostRedisplay();
}
