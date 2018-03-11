#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <time.h>
#include <SDL2\SDL.h>

#include "GL_framework.h"

int w, h; //variables donde guardamos la witdh y la height
float FOV_Augment=0.0f; //coeficiente que aumentara nuestro fov
int Do_Once = 0; //variable que controla nuestro dolly effect
clock_t  TimerDolly = 0.f; //tiempo de nuestro dolly effect
float RotationTimer = 0.0f; //tiempo que controla la rotacion de la camara


//Namespace que controla en que escena estamos y cuanto nos movemos dentro del mundo
namespace SceneControl {
	 bool scene1 = false;
	 bool scene2 = false;
	 bool scene2prt2 = false;
	 bool scene3 = false;

	 float Traveling = 0.f;
	 float LifterTime = 5.0f;
}

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Box {
	void setupCube();
	void cleanupCube();
	void drawCube();
}
namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}

//Dentro del namespace de cube tenemos nustras fuciones que nos permiten dibujar cuadrados
namespace Cube {
	void setupCube();
	void cleanupCube();
	void updateCube(const glm::mat4& transform);
	void drawCube();

	///-----------------------Scenes
	void Scene1(double currentime);//escena 1, contiene la composicion de la escena 
	void Scene23(double currentime);//escena 2 y escena 3 , contiene la composicion de la escena 


	void updateColor(const glm::vec4 newColor);
	glm::vec4 myColor = { 0.0f, 0.3f, 1.4f, 1.0f };
}



namespace RenderVars {
	float FOV = glm::radians(65.f); //Fov
	const float zNear = 2.f;
	const float zFar = 150.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

//Funcion que nos permite controlar los eventos de teclado, la llamamos en el MAIN
void myKeyController(SDL_Event eve) {//pasamos como parametro un evento SDL
	switch (eve.type) {
	case SDL_KEYDOWN://controlamos que se haya pulsado una tecla
		switch (eve.key.keysym.sym)
		{
		case SDLK_1://Si pulsamos la tecla 1
				SceneControl::scene1 = true;//vemos la escena 1
				SceneControl::scene2 = false;//no vemos la escena 2 ni la 3
				SceneControl::scene3 = false;
				SceneControl::Traveling = 0.f;//reinicimaos el traveling
				SceneControl::LifterTime = 5.0f;//reinicimaos su tiempo de vida
				break;
		case SDLK_2:
				SceneControl::scene1 = false;
				SceneControl::scene2 = true;
				SceneControl::scene3 = false;
				SceneControl::Traveling = 0.f;
				SceneControl::LifterTime = 5.0f;
				break;
		case SDLK_3:
				SceneControl::scene1 = false;
				SceneControl::scene2 = false;
				SceneControl::scene2prt2 = false;
				SceneControl::scene3 = true;
				SceneControl::Traveling = 0.f;
				SceneControl::LifterTime = 5.0f;
				break;
		}
	
	}
}


void myInitCode(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	w = width;//inicializamos nuestras variables witdh y height a sus valores correspondientes
	h = height;

	Box::setupCube();
	Axis::setupAxis();
	Cube::setupCube();
	Cube::setupCube();
}

void myCleanupCode() {
	Box::cleanupCube();
	Axis::cleanupAxis();
	Cube::cleanupCube();
	Cube::cleanupCube();
}

void myRenderCode(double currentTime) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));

	if (SceneControl::scene1) {//si estamos en la escena 1
		float aux = -50.f;
		RV::_projection = glm::ortho((float)-w / aux, (float)w / aux, (float)h / aux, (float)-h / aux, 0.1f, 100.f); //camara orthonormal
		RV::_modelView = glm::rotate(glm::mat4(1.0f), 170.f, glm::vec3(1.f, 1.f, 0.f));//rotamos para tener una perspectiva 3D
		RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0] + SceneControl::Traveling, RV::panv[1], RV::panv[2])); //aplicamos el traveling lateral
	}
	else
	{
		RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);//si no es la escena 1 tenemos vision en perspectiva
	}

	if (SceneControl::scene2)//Si queremos la escena 2
	{
		Do_Once = 0;//empezamos en 0 ya que aun no hemos repetido la escena
		if (SceneControl::scene2prt2)//se activa cuando llegamos a la parte B de la escena 2
		{
			RV::FOV = glm::radians(65.f + FOV_Augment);//aumentamos el fov progresivamente
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1] + 5, (RV::panv[2] + 15)));//trasladamos la camara a su posicion
			FOV_Augment += 0.1f;//aumentamos el fov
		}
		else//parte A de la escena 2
		{
			RV::FOV = glm::radians(65.f); //volvemos a poner el fov a 65 grad
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1] + 5, (RV::panv[2] + 15) + SceneControl::Traveling * 2)); //aplicamos el zoom
		}
	}

	if (SceneControl::scene3)//cuando activamos la escena 3
	{
		if (Do_Once == 0)//reiniciamos los valores para que cada vez que se repita empiece de 0
		{
			FOV_Augment = 0.f;
			Do_Once++;
		}

		if (SceneControl::Traveling >= 4.7f)//mientras tenga tiempo de vida haremos el dolly effect
		{

			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1] + 5, (RV::panv[2] + 15))); //movemos la camara
			SceneControl::Traveling = 0.f; //reiniciamos el traveling y el Fov
			FOV_Augment = 0.f;

			if (TimerDolly >= 5.f)//comprovamos que aun este haciendo el efecto
			{
				FOV_Augment = 0.f;
				TimerDolly = 0.f;
			}

		}
		else//aplicamos el dolly effect
		{
			FOV_Augment += 0.38; //aumentamos la variable del fov
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1] + 5, (RV::panv[2] + 15) + SceneControl::Traveling * 2));//nos trasladamos
			RV::FOV = glm::radians(65.f + FOV_Augment);//aumentamos el FOV
		}

	}

	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;


	// render code
	Box::drawCube();
	Axis::drawAxis();
	
	if (SceneControl::scene1)
	{
		if (SceneControl::Traveling >= 10.f)//comprobamos que la escena 1 no haya llegado a su limite de traveling
			SceneControl::Traveling = 0.0f;
		else
			SceneControl::Traveling += 0.02f; //aumentamos el traveling
	}
	if (SceneControl::scene2)
	{
		
		if (SceneControl::Traveling >= 10.f && !SceneControl::scene2prt2) {//comprobamos que la escena 2 haya llegado a su parte B
			SceneControl::Traveling = 0.0f;
			SceneControl::scene2prt2 = true;
		}
		else
			SceneControl::Traveling += 0.02f; //si no ha llegado nos vamos desplazando
	}
	if (SceneControl::scene2prt2)
	{

		if (SceneControl::Traveling >= 10.f) {//comprobamos si la parte B ha llegado a su fin
			SceneControl::Traveling = 0.0f;
			SceneControl::scene2 = true;
			SceneControl::scene2prt2 = false;
		}
		else
			SceneControl::Traveling += 0.02f;
	}
	if (SceneControl::scene3)
	{
			SceneControl::Traveling += 0.03f;//aumentamos el traveling en la escena 3
	}

	if (SceneControl::scene1)//comprobamos que escena estamos renderizando
		Cube::Scene1(currentTime);//renderiza la escena 1
	if (SceneControl::scene2 || SceneControl::scene3)//como la escena 2 y la 3 son la misma
		Cube::Scene23(currentTime);//renderiza la escena 2-3

	ImGui::Render();
}

GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

////////////////////////////////////////////////// BOX
namespace Box {
	GLuint cubeVao;
	GLuint cubeVbo[2];
	GLuint cubeShaders[2];
	GLuint cubeProgram;

	float cubeVerts[] = {
		// -5,0,-5 -- 5, 10, 5
		-5.f,  0.f, -5.f,
		5.f,  0.f, -5.f,
		5.f,  0.f,  5.f,
		-5.f,  0.f,  5.f,
		-5.f, 10.f, -5.f,
		5.f, 10.f, -5.f,
		5.f, 10.f,  5.f,
		-5.f, 10.f,  5.f,
	};
	GLubyte cubeIdx[] = {
		1, 0, 2, 3, // Floor - TriangleStrip
		0, 1, 5, 4, // Wall - Lines
		1, 2, 6, 5, // Wall - Lines
		2, 3, 7, 6, // Wall - Lines
		3, 0, 4, 7  // Wall - Lines
	};

	const char* vertShader_xform =
		"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* fragShader_flatColor =
		"#version 330\n\
out vec4 out_Color;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = color;\n\
}";

	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(2, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 20, cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(vertShader_xform, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(fragShader_flatColor, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(2, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void drawCube() {
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		// FLOOR
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.6f, 0.6f, 0.6f, 1.f);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
		// WALLS
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.f, 0.f, 0.f, 1.f);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 4));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 8));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 12));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 16));

		glUseProgram(0);
		glBindVertexArray(0);
	}
}


////////////////////////////////////////////////// AXIS
namespace Axis {
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};
	const char* Axis_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
in vec4 in_Color;\n\
out vec4 vert_color;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	vert_color = in_Color;\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* Axis_fragShader =
		"#version 330\n\
in vec4 vert_color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = vert_color;\n\
}";

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}
	void drawAxis() {
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}


////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};




	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";


	const char* cube_fragShader =
		"#version 330\n\
in vec4 vert_Normal;\n\
out vec4 out_Color;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );\n\
}";
	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawCube() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}
	//Funcion de renderizado de la escena 1
	void Scene1(double currentime) 
	{
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);

		glm::mat4 Mymatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.f, 2.f, 0.f));//ponemos el cubo en la posicion que queremos
		objMat = Mymatrix; 
		glm::vec4 newColor = { 12.0f, 0.0f + -12.f*(float)sin(currentime), 4.0f,1.0f };//creamos el color y lo updateamos
		Cube::updateColor(newColor);

		glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(1.f + (float)sin(currentime)-0.5f, 1.f + (float)sin(currentime)-0.5f, 1.f));//escalamos el cubo en dufuncion del tiempo
		glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(2, 4.f, 1.f));//lo trasladamos
		objMat = t*s; //aplicamos las transformaciones

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);	

		//-------------------------------------------------------------------------------------------- 
	
		glm::vec4 newColor1 = { (float)sin(SceneControl::Traveling), (float)cos(SceneControl::Traveling), (float)sin(SceneControl::Traveling),1.0f };
		Cube::updateColor(newColor);

		glm::mat4 tr = glm::translate(glm::mat4(1.0f), glm::vec3(2.f, 10.f, 0.f));//posicionamos dentro del mundo
		glm::mat4 r = glm::rotate(glm::mat4(1.0f), 5.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 5.f, 0.f));//matriz de rotacion
		objMat = r*tr;//aplicamos la transformacion

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), (float)sin(SceneControl::Traveling), 1.f, (float)sin(SceneControl::Traveling), 0.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		//--------------------------------------------------------------------------------------------

		glm::mat4 q = glm::scale(glm::mat4(1.0f), glm::vec3(sin(currentime), 2.f, 0.7f));//matriz de escalado
		glm::mat4 y = glm::translate(glm::mat4(1.0f), glm::vec3(-4.f, 3.f, -5.f));//matriz de traslacion
		objMat = y*q; //aplicamos la transformacion

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		 q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		 y = glm::translate(glm::mat4(1.0f), glm::vec3(-8, sin(currentime), -5.f));
		objMat = y * q;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		 q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		 y = glm::translate(glm::mat4(1.0f), glm::vec3(-9, cos(currentime), -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);

		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(-10, sin(currentime), -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(-11, cos(currentime), -5.f));
		objMat = y * q;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(-12, sin(currentime), -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(-13, cos(currentime), -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		//--------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(sin(currentime),0 , -5.f));
		objMat = y * q;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(cos(currentime), 1, -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(sin(currentime), 2, -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(cos(currentime), 3, -5.f));
		objMat = y * q;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(sin(currentime), 4, -5.f));
		objMat = y * q; 

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		q = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 1.f, 0.7f));
		y = glm::translate(glm::mat4(1.0f), glm::vec3(cos(currentime), 5, -5.f));
		objMat = y * q;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//---------------------------------------------------------------------------------------------

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void Scene23(double currentime) {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);

		glm::mat4 Mymatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.f, 2.f, 1.f));//posicionamos en el mundo
		objMat = Mymatrix;
		glm::vec4 newColor = { 2.0f, 8.0f + -12.f*(float)sin(currentime), 1.0f,1.0f };//nuevo color
		Cube::updateColor(newColor);//updateamos el color

		glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(SceneControl::Traveling, 4.f, -4.f));//trasladamos en funcion del traveling
		objMat = t;//aplicamos la transformacion

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)//dependiendo del tiempo rotar hacia un lado u el otro
		{
			glm::mat4 r = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));//matriz de rotacion
			objMat = t*r;//aplicamos la transofrmacion
		}


		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		Cube::updateColor(newColor);
		glm::mat4 t2 = glm::translate(glm::mat4(1.0f), glm::vec3(-SceneControl::Traveling, 5.f, -4.f));
		objMat = t2;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r2 = glm::rotate(glm::mat4(1.0f), -20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t2 * r2;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t3 = glm::translate(glm::mat4(1.0f), glm::vec3(SceneControl::Traveling, 6.f, -4.f));
		objMat = t3;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r3 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t3 * r3;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t4 = glm::translate(glm::mat4(1.0f), glm::vec3(-SceneControl::Traveling, 3.f, -4.f));
		objMat = t4;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r4 = glm::rotate(glm::mat4(1.0f), -20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t4 * r4;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t5 = glm::translate(glm::mat4(1.0f), glm::vec3(SceneControl::Traveling, 2.f, -4.f));
		objMat = t5;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r5 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t5 * r5;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t6 = glm::translate(glm::mat4(1.0f), glm::vec3(-SceneControl::Traveling, 1.f, -4.f));
		objMat = t6;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r6 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t6 * r6;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t7 = glm::translate(glm::mat4(1.0f), glm::vec3(-SceneControl::Traveling, 7.f, -4.f));
		objMat = t7;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r7 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t7 * r7;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor);
		glm::mat4 t8 = glm::translate(glm::mat4(1.0f), glm::vec3(SceneControl::Traveling, 8.f, -4.f));
		objMat = t8;

		if ((int)currentime % 3 == 1 || (int)currentime % 3 == 0 && (int)currentime != 0)
		{
			glm::mat4 r8 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
			objMat = t8 * r8;
		}

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		glm::vec4 newColor2 = { 0.0f, 15.0f + -3.f*(float)sin(currentime), 14.0f,1.0f };
		Cube::updateColor(newColor2);
		glm::mat4 t9 = glm::translate(glm::mat4(1.0f), glm::vec3(19.f, 2.f, -1.f));
		glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		glm::mat4 r10 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
		objMat = t9*s*r10;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		Cube::updateColor(newColor2);
		glm::mat4 t10 = glm::translate(glm::mat4(1.0f), glm::vec3(-19.f, 2.f, -1.f));
		glm::mat4 s2 = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		glm::mat4 r9 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
		objMat = t10 * s2*r9;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		Cube::updateColor(newColor2);
		glm::mat4 t11 = glm::translate(glm::mat4(1.0f), glm::vec3(-19.f, 6.f, -1.f));
		glm::mat4 s3 = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		glm::mat4 r11 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
		objMat = t11 * s3 *r11;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		Cube::updateColor(newColor2);
		glm::mat4 t12 = glm::translate(glm::mat4(1.0f), glm::vec3(19.f, 6.f, -1.f));
		glm::mat4 s4 = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		glm::mat4 r12 = glm::rotate(glm::mat4(1.0f), 20.f*float(sin(SceneControl::Traveling)), glm::vec3(0.f, 0.f, 1.f));
		objMat = t12 * s4 *r12;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		glm::vec4 newColor3 = { 1.f + -0.5f*(float)sin(currentime), 1.0f + -5.f*(float)sin(currentime), 1.0f,1.0f };
		Cube::updateColor(newColor3);
		glm::mat4 t13 = glm::translate(glm::mat4(1.0f), glm::vec3(4.f, 6.f, -10.f));
		glm::mat4 s5 = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		objMat = t13 * s5;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		Cube::updateColor(newColor3);
		glm::mat4 t14 = glm::translate(glm::mat4(1.0f), glm::vec3(-4.f, 6.f, -10.f));
		glm::mat4 s6 = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 1.f));
		objMat = t14 * s6;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t15= glm::translate(glm::mat4(1.0f), glm::vec3(0, 5.f, 3.f));
		objMat = t15;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t16 = glm::translate(glm::mat4(1.0f), glm::vec3(6.f, 5.f, 0.f));
		glm::mat4 s7 = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 1.f, 3.f));
		objMat = t16 * s7;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t17 = glm::translate(glm::mat4(1.0f), glm::vec3(-6.f, 5.f, 0.f));
		glm::mat4 s8 = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 1.f, 3.f));
		objMat = t17 * s8;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t18 = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 2.f, -6.f));
		glm::mat4 s9 = glm::scale(glm::mat4(1.0f), glm::vec3(5.f, 1.f, 1.f));
		objMat = t18 * s9;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t19 = glm::translate(glm::mat4(1.0f), glm::vec3(-5.f, 9.f, -16.f));
		glm::mat4 s10 = glm::scale(glm::mat4(1.0f), glm::vec3(5.f, 1.f, 1.f));
		objMat = t19 * s10;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------


		glm::mat4 t20 = glm::translate(glm::mat4(1.0f), glm::vec3(5.f, 9.f, -16.f));
		glm::mat4 s11 = glm::scale(glm::mat4(1.0f), glm::vec3(5.f, 1.f, 1.f));
		objMat = t20 * s11;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------
		glm::mat4 t21 = glm::translate(glm::mat4(1.0f), glm::vec3(-7.f, 8.f, 0.f));
		glm::mat4 s12 = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 1.f, 3.f));
		objMat = t21 * s12;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glm::mat4 t22 = glm::translate(glm::mat4(1.0f), glm::vec3(7.f, 8.f, 0.f));
		glm::mat4 s13 = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 1.f, 3.f));
		objMat = t22 * s13;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.r, myColor.g, myColor.b, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//--------------------------------------------------------------------------------------------

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void updateColor(const glm::vec4 newColor) {
		myColor = newColor;
	}
}

