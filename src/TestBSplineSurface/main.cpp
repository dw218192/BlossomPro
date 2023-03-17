#include "BsplineSurface.h"

#include <iostream>;
#include <vector>;
#include <format>;
#include <stdexcept>;
#include <array>;
#include <cmath>;
#include <numbers>;

#include <GL/glut.h>
#include <glm/gtc/constants.hpp>
struct graphics
{
	static const double LENGTH;
	static const double oneOverSquareRoot2;
	double tractionAngle{};
	bool leftMouseButtonPressed{};
	bool rightMouseButtonPressed{};
	int lastX{}, lastY{};
	double zoomScale{ LENGTH };
	double aspectRatio{ 1.0 };
	double xTranslation{}, yTranslation{};
	double sNear{ -LENGTH }, sFar{ LENGTH };
	std::array<double, 3> currentVec{}, prevVec{}, rotationAxis{};
	GLdouble mxTransform[4][4]{ {-0.7071, -0.5, 0.5, 0.0}, {0.7071, -0.5, 0.5, 0.0}, {0.0, 0.7071, 0.7071, 0.0}, {0.0, 0.0, 0.0, 1.0} }; // isometric view
};

const double graphics::LENGTH{ 150.0 };
const double graphics::oneOverSquareRoot2{ 1.0 / sqrt(2.0) };

graphics g{};

BsplineSurface bs00{ 3, 3 };

void ptTo3DVec(int x, int y, std::array<double, 3>& vec)
{
	// x^2 + y^2 + z^2 == r^2, (r == 1)

	int w{ glutGet(GLUT_WINDOW_WIDTH) };
	int h{ glutGet(GLUT_WINDOW_HEIGHT) };
	//std::cout << std::format("width: {}, heigh: {}\n", w, h);

	vec[0] = 2.0 * x / w - 1.0;
	vec[1] = -2.0 * y / h + 1.0;
	double hypot{ std::hypot(vec[0], vec[1]) };

	if (hypot <= g.oneOverSquareRoot2) // x^2 + y^2 <= r^2 / 2
	{
		vec[2] = sqrt(1.0 - hypot * hypot); // z == sqrt(r^2 - (x^2 + y^2))
	}
	else
	{
		vec[2] = 0.5 / hypot; // z == (r^2 / 2) / sqrt(x^2 + y^2)
	}

	hypot = std::hypot(vec[0], vec[1], vec[2]);

	vec[0] /= hypot;
	vec[1] /= hypot;
	vec[2] /= hypot;

	//std::cout << std::format("vector: {}, {}, {}\n", vec[0], vec[1], vec[2]);
}

void onKeyStroke(unsigned char key, int x, int y)
{
	if (key == 'r' || key == 'R')
	{
		g.zoomScale = graphics::LENGTH;
		g.mxTransform[0][0] = -0.7071;
		g.mxTransform[0][1] = -0.5;
		g.mxTransform[0][2] = 0.5;
		g.mxTransform[0][3] = 0.0;
		g.mxTransform[1][0] = 0.7071;
		g.mxTransform[1][1] = -0.5;
		g.mxTransform[1][2] = 0.5;
		g.mxTransform[1][3] = 0.0;
		g.mxTransform[2][0] = 0.0;
		g.mxTransform[2][1] = 0.7071;
		g.mxTransform[2][2] = 0.7071;
		g.mxTransform[2][3] = 0.0;
		g.mxTransform[3][0] = 0.0;
		g.mxTransform[3][1] = 0.0;
		g.mxTransform[3][2] = 0.0;
		g.mxTransform[3][3] = 1.0;
		g.xTranslation = g.yTranslation = 0.0;
		glutPostRedisplay();
	}
}

void onMouseButton(int button, int state, int x, int y)
{
	//std::cout << std::format("button: {}, state: {}, x: {}, y: {}\n", button, state, x, y);
	if (button == 0 && state == 0) // left mouse button pressed
	{
		g.leftMouseButtonPressed = true;
		ptTo3DVec(x, y, g.prevVec);
	}
	else if (button == 0 && state == 1) // left mouse button released
	{
		g.leftMouseButtonPressed = false;
	}
	else if (button == 2 && state == 0) // right mouse button pressed
	{
		g.rightMouseButtonPressed = true;
		g.lastX = x;
		g.lastY = y;
	}
	else if (button == 2 && state == 1) // right mouse button released
	{
		g.rightMouseButtonPressed = false;
	}
	else if (button == 3 && state == 0) // scroll forward
	{
		//std::cout << "scroll forward\n";
		g.zoomScale *= 0.9;
		glutPostRedisplay();
	}
	else if (button == 4 && state == 0) // scroll backward
	{
		//std::cout << "scroll backward\n";
		g.zoomScale *= 1.1;
		glutPostRedisplay();
	}
}

void onMouseDrag(int x, int y)
{
	if (g.leftMouseButtonPressed)
	{
		ptTo3DVec(x, y, g.currentVec);
		//std::cout << std::format("x: {}, y: {}\n", x, y);

		double innerProduct{ g.currentVec[0] * g.prevVec[0] + g.currentVec[1] * g.prevVec[1] + g.currentVec[2] * g.prevVec[2] };
		innerProduct = std::min(innerProduct, 1.0);
		g.tractionAngle = 180.0 * std::acos(innerProduct) / glm::pi<double>(); // in degree
		//std::cout << std::format("angle: {}\n", tractionAngle);

		g.rotationAxis[0] = g.prevVec[1] * g.currentVec[2] - g.prevVec[2] * g.currentVec[1];
		g.rotationAxis[1] = g.prevVec[2] * g.currentVec[0] - g.prevVec[0] * g.currentVec[2];
		g.rotationAxis[2] = g.prevVec[0] * g.currentVec[1] - g.prevVec[1] * g.currentVec[0];

		//std::cout << std::format("axis: {}, {}, {}\n", rotationAxis[0], rotationAxis[1], rotationAxis[2]);

		g.prevVec = g.currentVec;

		glutPostRedisplay();
	}
	else if (g.rightMouseButtonPressed)
	{
		g.xTranslation += static_cast<double>(g.lastX - x) * 2.0 * g.zoomScale / glutGet(GLUT_WINDOW_WIDTH);
		g.yTranslation += static_cast<double>(y - g.lastY) * 2.0 * g.zoomScale / glutGet(GLUT_WINDOW_HEIGHT);
		//std::cout << std::format("xTranlation: {}, yTranslation: {}\n", xTranslation, yTranslation);
		g.lastX = x;
		g.lastY = y;
		glutPostRedisplay();
	}
}

void reshape(int x, int y)
{
	g.aspectRatio = static_cast<double>(y) / x; // the inverse of aspect ratio
	glViewport(0, 0, x, y);
}

void display()
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-g.zoomScale + g.xTranslation, g.zoomScale + g.xTranslation, -g.zoomScale * g.aspectRatio + g.yTranslation, g.zoomScale * g.aspectRatio + g.yTranslation, g.sNear, g.sFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (g.leftMouseButtonPressed)
	{
		glPushMatrix();

		glLoadIdentity();
		glRotated(g.tractionAngle, g.rotationAxis[0], g.rotationAxis[1], g.rotationAxis[2]);
		glMultMatrixd(&g.mxTransform[0][0]);
		glGetDoublev(GL_MODELVIEW_MATRIX, &g.mxTransform[0][0]);

		glPopMatrix();
	}

	glMultMatrixd(&g.mxTransform[0][0]);

	glBegin(GL_LINES);
	// x-axis
	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(100.0, 0.0, 0.0);

	// y-axis
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 100.0, 0.0);

	// z-axis
	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 100.0);
	glEnd();

	// 'x'
	double tx{ 100.0 * g.mxTransform[0][0] + g.mxTransform[3][0] };
	double ty{ 100.0 * g.mxTransform[0][1] + g.mxTransform[3][1] };
	double tz{ 100.0 * g.mxTransform[0][2] + g.mxTransform[3][2] };

	glColor3d(1.0, 0.0, 0.0);

	glPushMatrix();
	glLoadIdentity();
	glTranslated(tx, ty, tz);
	glScaled(0.1, 0.1, 0.1);
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'x');
	glPopMatrix();

	// 'y'
	tx = 100.0 * g.mxTransform[1][0] + g.mxTransform[3][0];
	ty = 100.0 * g.mxTransform[1][1] + g.mxTransform[3][1];
	tz = 100.0 * g.mxTransform[1][2] + g.mxTransform[3][2];

	glColor3d(0.0, 1.0, 0.0);

	glPushMatrix();
	glLoadIdentity();
	glTranslated(tx, ty, tz);
	glScaled(0.1, 0.1, 0.1);
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'y');
	glPopMatrix();

	// 'z'
	tx = 100.0 * g.mxTransform[2][0] + g.mxTransform[3][0];
	ty = 100.0 * g.mxTransform[2][1] + g.mxTransform[3][1];
	tz = 100.0 * g.mxTransform[2][2] + g.mxTransform[3][2];

	glColor3d(0.0, 0.0, 1.0);

	glPushMatrix();
	glLoadIdentity();
	glTranslated(tx, ty, tz);
	glScaled(0.1, 0.1, 0.1);
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'z');
	glPopMatrix();

	glColor3d(1.0, 1.0, 1.0);
	glm::vec3 pt;
	float size = 5.f;
	for (int u{}; u <= size; ++u)
	{
		glBegin(GL_LINE_STRIP); // glBegin(GL_POINTS);
		for (int v{}; v <= size; ++v)
		{
			bs00.surfacePoint(u / size, v / size, pt);
			//std::cout << std::format("u: {:15.5f}, v: {:15.5f}, ({:15.5f}, {:15.5f}, {:15.5f})\n", u, v, pt.x, pt.y, pt.z);
			glVertex3d(pt.x, pt.y, pt.z);
		}
		glEnd();
	}

	for (int u{}; u <= size; ++u)
	{
		glBegin(GL_LINE_STRIP); // glBegin(GL_POINTS);
		for (int v{}; v <= size; ++v)
		{
			bs00.surfacePoint(v / size, u / size, pt);
			glVertex3d(pt.x, pt.y, pt.z);
		}
		glEnd();
	}

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	try
	{
		//std::vector<Point3D> vp0{ {50, -30, 10}, {20, -30, 10}, {0, -30, 30}, {-20, -30, 30} };
		//std::vector<Point3D> vp1{ {50, 10, 10}, {20, 10, 10}, {0, 10, 30}, {-20, 10, 30} };
		//std::vector<Point3D> vp2{ {50, 30, 0}, {20, 30, 0}, {0, 30, 10}, {-20, 30, 10} };
		//std::vector<Point3D> vp3{ {50, 50, 0 }, { 20, 50, 0 }, {0, 50, 10}, {-20, 50, 10} };
		//std::vector<Point3D> vp4{ {50, 60, 0}, {20, 60, 0}, {0, 60, 10}, {-20, 60, 10} };

		//bs00.addVector(vp0);
		//bs00.addVector(vp1);
		//bs00.addVector(vp2);
		//bs00.addVector(vp3);
		//bs00.addVector(vp4);

		std::vector<glm::vec3> vp0{ {50, -30, 10}, {20, -30, 10}, {0, -30, 30}, {-20, -30, 30} };
		std::vector<glm::vec3> vp1{ {50, 10, 10}, {20, 10, 10}, {0, 10, 30}, {-20, 10, 30} };
		std::vector<glm::vec3> vp2{ {50, 30, 0}, {20, 30, 0}, {0, 30, 10}, {-20, 30, 10} };
		std::vector<glm::vec3> vp3{ {50, 50, 0 }, { 20, 50, 0 }, {0, 50, 10}, {-20, 50, 10} };

		bs00.addVector(vp0);
		bs00.addVector(vp1);
		bs00.addVector(vp2);
		bs00.addVector(vp3);

		bs00.makeKnots();

		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
		glutInitWindowSize(600, 600);
		glutCreateWindow("B-spline Surface");
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutMouseFunc(onMouseButton);
		glutMotionFunc(onMouseDrag);
		glutKeyboardFunc(onKeyStroke);
		glutMainLoop();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	catch (...)
	{
		std::cerr << "something wrong\n";
		return 1;
	}
}
