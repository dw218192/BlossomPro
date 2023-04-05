#include "KeyframeCurveWidget.h"
#include <QPainter>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using kfcw = KeyframeCurveWidget;

float const kfcw::MY_PI = 3.1415926f;

kfcw::KeyframeCurveWidget(QWidget* parent /* = 0 */)
	: QOpenGLWidget(parent), m_axis(0), lastPos(0, 0),
	ctrlSelected(-1), m_bIfModify(false), m_spline_data(0),
	m_CameraPos{ 5, 5, 0 },
	m_deltaT(0.1) {
	setFocusPolicy(Qt::ClickFocus);
}

void kfcw::setAE(aaAaa::aaSpline* sdata) {
	m_spline_data = sdata;
	ctrlSelected = -1;
	update();
	emit selectValuesChanged(0, 0);
}

static void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
	out[0] =
		M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
	out[1] =
		M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
	out[2] =
		M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
	out[3] =
		M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}
static GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
	const GLdouble model[16], const GLdouble proj[16],
	const GLint viewport[4],
	GLdouble* winx, GLdouble* winy, GLdouble* winz)
{
	GLdouble in[4], out[4];

	in[0] = objx;
	in[1] = objy;
	in[2] = objz;
	in[3] = 1.0;
	transformPoint(out, model, in);
	transformPoint(in, proj, out);

	if (in[3] == 0.0)
		return GL_FALSE;

	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];

	*winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
	*winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

	*winz = (1 + in[2]) / 2;
	return GL_TRUE;
}

void kfcw::renderText(double x, double y, double z, QString text)
{
	int width = this->width();
	int height = this->height();

	GLdouble model[4][4], proj[4][4];
	GLint view[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
	glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
	glGetIntegerv(GL_VIEWPORT, &view[0]);
	GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

	project(x, y, z,
		&model[0][0], &proj[0][0], &view[0],
		&textPosX, &textPosY, &textPosZ);

	textPosY = height - textPosY; // y is inverted

	QPainter painter(this);
	painter.setPen(Qt::yellow);
	painter.setFont(QFont("Helvetica", 8));
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.drawText(textPosX, textPosY, text); // z = pointT4.z + distOverOp / 4
	painter.end();
}

void kfcw::drawGrid() {
	int gridSize = 10;
	float gridSpacing = 1.0f;

	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);

	for (int i = -gridSize; i <= gridSize; ++i)
	{
		glVertex3f(i * gridSpacing, 0.0f, -gridSize * gridSpacing);
		glVertex3f(i * gridSpacing, 0.0f, gridSize * gridSpacing);

		glVertex3f(-gridSize * gridSpacing, 0.0f, i * gridSpacing);
		glVertex3f(gridSize * gridSpacing, 0.0f, i * gridSpacing);
	}

	glEnd();
}

void kfcw::drawSpline() {
	if (!m_spline_data)
		return;
	aaAaa::aaSpline& m_spline_data = *(this->m_spline_data);

	aaAaa::aaCurvePtr pspline;
	if (m_spline_data.size() <= 1) {
		return;
	}
	else {
		pspline = aaAaa::aaCurveFactory::createCurve(m_spline_data);
		if (!pspline.get())
			return;
	}

	if (m_spline_data.bLimited) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
		glVertex3f(m_spline_data.limit_left, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
		glVertex3f(m_spline_data.limit_right, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
		glVertex3f(m_spline_data.limit_right, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
		glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
		glEnd();
	}


	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_POINTS);
	aaAaa::aaSpline::KnotsList::iterator cit = m_spline_data.knots.begin();
	aaAaa::aaSpline::KnotsList::iterator cend = m_spline_data.knots.end();
	for (; cit != cend; ++cit) {
		glVertex3f((*cit).t, (*cit).y * Y_FACTOR, Z_VALUE);
	}
	glEnd();

	if (ctrlSelected >= 0 && ctrlSelected < m_spline_data.knots.size()) {
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_POINTS);
		glVertex3f(m_spline_data.knots[ctrlSelected].t, m_spline_data.knots[ctrlSelected].y * Y_FACTOR, Z_VALUE);
		glEnd();
	}

	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_LINE_STRIP);

	aaAaa::aaSpline::KnotsList::iterator beg = m_spline_data.knots.begin();
	aaAaa::aaSpline::KnotsList::reverse_iterator rbeg = m_spline_data.knots.rbegin();

	double t = (*beg).t;
	double v = (*beg).y;
	glVertex3f(t, v * Y_FACTOR, Z_VALUE);
	t += m_deltaT;

	while (t < (*rbeg).t - m_deltaT) {
		pspline->getValue(t, v);
		if (m_spline_data.bLimited) {
			if (v > m_spline_data.limit_top)
				v = m_spline_data.limit_top;
			else if (v < m_spline_data.limit_bottom)
				v = m_spline_data.limit_bottom;
		}
		glVertex3f(t, v * Y_FACTOR, Z_VALUE);
		t += m_deltaT;
	}

	t = (*rbeg).t;
	v = (*rbeg).y;
	glVertex3f(t, v * Y_FACTOR, Z_VALUE);

	glEnd();


}

void kfcw::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(0.7, 0.7, 0.7, 1);
	glShadeModel(GL_FLAT);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);
	glPointSize(7.0);
}

void kfcw::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	int width = this->width();
	int height = this->height();
	// Load a perspective projection using glm
	float aspectRatio = static_cast<float>(width) / height;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), aspectRatio, 0.1f, 100.f);

	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(projectionMatrix));
	
	glMatrixMode(GL_MODELVIEW);
	glm::mat4 viewMatrix = glm::lookAt(m_CameraPos, m_CameraPos + glm::vec3{0,0,Z_VALUE}, glm::vec3{ 0,1,0 });
	
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	drawGrid();
	// drawSpline();
}

void kfcw::resizeGL(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	// Load a perspective projection using glm
	float aspectRatio = static_cast<float>(width) / height;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), aspectRatio, 0.1f, 100.f);

	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(projectionMatrix));
}

void kfcw::wheelEvent(QWheelEvent* e)
{
	if (e->delta() > 0)
		m_CameraPos.z -= 3;
	else if (e->delta() < 0)
		m_CameraPos.z += 3;
	if (m_CameraPos.z < 0)
		m_CameraPos.z = 0;

	update();
}