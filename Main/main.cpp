
#include <GL/glew.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <GL/glew.h>
#include <Live2D.h>
#include <util/UtSystem.h>
#include <util/Json.h>
#include <Live2DFramework.h>
#include <assert.h>
#include "LAppModel.h"
#include "PlatformManager.h"

using namespace std;

LAppModel* live2DModel;

void display(void)
{
	//glClearColor(0.0, 0.0, 1.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);

	live2DModel->update();
	live2DModel->draw();


	glFlush();
}

void timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(30, timer, 0);
}

void Live2DInit(void)
{
	glClearColor(0.0, 0.0, 1.0, 1.0);

	live2d::framework::Live2DFramework::setPlatformManager(new PlatformManager());

	std::string path = "res/kasumi/kasumi.model.json";
	live2DModel = new LAppModel();

	live2DModel->load(path.c_str());
}

void resize(int w, int h)
{
	glViewport(0, 0, w, h);

	glLoadIdentity();

	live2DModel->resize(w, h);
}


int main(int argc, char *argv[])
{
	live2d::Live2D::init();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Model");

	glewInit();
	Live2DInit();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutTimerFunc(100, timer, 0);
	glutMainLoop();
	
	return 0;
}

