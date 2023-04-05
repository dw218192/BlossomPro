#include "KeyframeCurveWidget.h"
#include <QPainter>
#include <QtWidgets>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using kfcw = KeyframeCurveWidget;

static constexpr float k_zoomFactor = 0.9;
static constexpr float k_numGrids = 10;
static constexpr float k_gridSpacing = 1 / k_numGrids;

kfcw::KeyframeCurveWidget(QWidget* parent /* = 0 */)
	: QOpenGLWidget(parent),
	m_viewMax{ 1, 1 },
	m_camViewScale{ 1, 1 },
	m_camPos{ 0, 0 },
	m_lastPos{ 0, 0 },
	ctrlSelected(-1), m_bIfModify(false), m_spline_data(0),
	m_deltaT(0.1) {
	setFocusPolicy(Qt::ClickFocus);
}

void kfcw::setAE(aaAaa::aaSpline* sdata) {
	m_spline_data = sdata;
	ctrlSelected = -1;
	update();
	emit selectValuesChanged(0, 0);
}

void kfcw::renderText(glm::ivec2 screenPos, QString text)
{
	QPainter painter(this);
	painter.begin(this);
	painter.setPen(Qt::yellow);
	painter.setFont(QFont("Helvetica", 8));
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.drawText(screenPos.x, screenPos.y, text);
	painter.end();
}

void kfcw::drawGrid() {
	glBegin(GL_LINES);

	// X
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.1f);
	glVertex3f(1.0f, 0, 0.1f);

	// Y
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.1f);
	glVertex3f(0.0f, 1.0f, 0.1f);

	// Draw a grid on the X-Y plane
	for (int i = 0; i <= k_numGrids; ++i) {
		if (i > 0) {
			glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3f(0, i * k_gridSpacing, 0);
			glVertex3f(1, i * k_gridSpacing, 0);

			glVertex3f(i * k_gridSpacing, 0, 0);
			glVertex3f(i * k_gridSpacing, 1, 0);
		}

		// Render unit labels
		renderText(world2screen({ i * k_gridSpacing, 0.1 }), QString::number(i));
		renderText(world2screen({ 0.1, i * k_gridSpacing }), QString::number(i));
	}

	renderText(world2screen({ 0.5f, 0.5f }), "Test");

	glEnd();
}


void kfcw::drawSpline() {
	//if (!m_spline_data)
	//	return;
	//aaAaa::aaSpline& m_spline_data = *(this->m_spline_data);

	//aaAaa::aaCurvePtr pspline;
	//if (m_spline_data.size() <= 1) {
	//	return;
	//}
	//else {
	//	pspline = aaAaa::aaCurveFactory::createCurve(m_spline_data);
	//	if (!pspline.get())
	//		return;
	//}

	//if (m_spline_data.bLimited) {
	//	glColor3f(1.0f, 0.0f, 0.0f);
	//	glBegin(GL_LINE_STRIP);
	//	glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
	//	glVertex3f(m_spline_data.limit_left, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
	//	glVertex3f(m_spline_data.limit_right, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
	//	glVertex3f(m_spline_data.limit_right, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
	//	glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
	//	glEnd();
	//}


	//glColor3f(0.0f, 1.0f, 0.0f);
	//glBegin(GL_POINTS);
	//aaAaa::aaSpline::KnotsList::iterator cit = m_spline_data.knots.begin();
	//aaAaa::aaSpline::KnotsList::iterator cend = m_spline_data.knots.end();
	//for (; cit != cend; ++cit) {
	//	glVertex3f((*cit).t, (*cit).y * Y_FACTOR, Z_VALUE);
	//}
	//glEnd();

	//if (ctrlSelected >= 0 && ctrlSelected < m_spline_data.knots.size()) {
	//	glColor3f(1.0f, 1.0f, 0.0f);
	//	glBegin(GL_POINTS);
	//	glVertex3f(m_spline_data.knots[ctrlSelected].t, m_spline_data.knots[ctrlSelected].y * Y_FACTOR, Z_VALUE);
	//	glEnd();
	//}

	//glColor3f(1.0f, 0.0f, 1.0f);
	//glBegin(GL_LINE_STRIP);

	//aaAaa::aaSpline::KnotsList::iterator beg = m_spline_data.knots.begin();
	//aaAaa::aaSpline::KnotsList::reverse_iterator rbeg = m_spline_data.knots.rbegin();

	//double t = (*beg).t;
	//double v = (*beg).y;
	//glVertex3f(t, v * Y_FACTOR, Z_VALUE);
	//t += m_deltaT;

	//while (t < (*rbeg).t - m_deltaT) {
	//	pspline->getValue(t, v);
	//	if (m_spline_data.bLimited) {
	//		if (v > m_spline_data.limit_top)
	//			v = m_spline_data.limit_top;
	//		else if (v < m_spline_data.limit_bottom)
	//			v = m_spline_data.limit_bottom;
	//	}
	//	glVertex3f(t, v * Y_FACTOR, Z_VALUE);
	//	t += m_deltaT;
	//}

	//t = (*rbeg).t;
	//v = (*rbeg).y;
	//glVertex3f(t, v * Y_FACTOR, Z_VALUE);

	//glEnd();


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
	resizeGL(width(), height());
	drawGrid();
	// drawSpline();
}

void kfcw::resizeGL(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(
		m_camPos.x, // left
		m_camPos.x + glm::min(1.f, m_camViewScale.x), // right
		m_camPos.y, // bottom
		m_camPos.y + m_camViewScale.y, // top
		-1.0, 1.0
	);
}

glm::vec2 kfcw::screen2world(glm::ivec2 screenPos) {
	float w = this->width();
	float h = this->height();
	return glm::vec2{ screenPos.x / w, 1.0f - screenPos.y / h };
}

glm::ivec2 kfcw::world2screen(glm::vec2 world) {
	float w = this->width();
	float h = this->height();
	return glm::ivec2{ world.x * w, h - world.y * h };
}

void kfcw::mousePressEvent(QMouseEvent* event) {

}

void kfcw::mouseReleaseEvent(QMouseEvent* event) {

}

void kfcw::mouseMoveEvent(QMouseEvent* event) {

}

void kfcw::wheelEvent(QWheelEvent* e)
{
	if (e->delta() > 0)
		m_camViewScale /= k_zoomFactor;
	else if (e->delta() < 0)
		m_camViewScale *= k_zoomFactor;
	m_camViewScale = glm::clamp(m_camViewScale, { 0.1f, 0.1f }, { 1000.0f, 1000.0f });

	update();
}