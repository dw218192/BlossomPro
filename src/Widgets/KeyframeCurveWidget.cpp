#include "KeyframeCurveWidget.h"
#include <QtWidgets>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// convention

// view volume size is an orthographic volume defined by [view_min, view_max]
// world grid size = [0-1, 0-1]
// zoom in/out is implemented by view volume size change

static constexpr double k_zoomFactor = 0.9;
static constexpr double k_gridSpacing = 0.1;
static constexpr double k_editTolerance = 0.05;
static constexpr double k_evalStep = 0.01;

using kfcw = KeyframeCurveWidget;

kfcw::KeyframeCurveWidget(QWidget* parent, SplineType type)
	: QOpenGLWidget(parent),
	m_yScale(1.0),
	m_viewMin{ 0,0 },
	m_viewMax{ 1,1 },
	m_lastPos{ 0,0 },
	m_curEdit{ std::nullopt }
{
	m_func.addControlPoint(0, 0);
	m_func.addControlPoint(1, 1);
	m_func.setType(SplineType::Linear);
	m_func.setYScale(m_yScale);

	setFocusPolicy(Qt::ClickFocus);
}

void kfcw::setYScale(double scale) {
	m_yScale = glm::clamp(scale, 1.0, 1000.0);
	m_func.setYScale(m_yScale);

	update();
}

void kfcw::setSplineType(SplineType type) {
	m_func.setType(type);
	update();
}

void kfcw::renderText(QPainter& painter, glm::ivec2 screenPos, QString text)
{
	painter.setPen(Qt::yellow);
	painter.setFont(QFont("Helvetica", 8));
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.drawText(screenPos.x, screenPos.y, text);
}

void kfcw::drawGrid() {
	static constexpr v2 maxPoint{ 1, 1 };
	static constexpr v2 minPoint{ 0, 0 };

	glBegin(GL_LINES);
	// X
	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(minPoint.x, 0, 0.1);
	glVertex3d(maxPoint.x, 0, 0.1);

	// Y
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(0.0, minPoint.y, 0.1);
	glVertex3d(0.0, maxPoint.y, 0.1);

	// Draw a grid on the X-Y plane
	glColor3d(0.5, 0.5, 0.5);
	for (double y = 0; y <= maxPoint.y + k_gridSpacing; y += k_gridSpacing) {
		glVertex3d(minPoint.x, y, 0);
		glVertex3d(maxPoint.x, y, 0);
	}
	for (double x = 0; x <= maxPoint.x + k_gridSpacing; x += k_gridSpacing) {
		glVertex3d(x, minPoint.y, 0);
		glVertex3d(x, maxPoint.y, 0);
	}
	glEnd();

	QPainter painter{ this };
	// Draw labels, which are view dependent
	for (double y = 0; y <= maxPoint.y + k_gridSpacing; y += k_gridSpacing) {
		iv2 pos = world2screen({ 0, y });
		pos.x = 0;
		renderText(painter, pos, QString::number(y * m_yScale, 'f', 1));
	}
	for (double x = 0; x <= maxPoint.x + k_gridSpacing; x += k_gridSpacing) {
		iv2 pos = world2screen({ x, 0 });
		pos.y = height();
		renderText(painter, pos, QString::number(x, 'f', 1));
	}
	painter.end();
}


void kfcw::drawSpline() {
	auto& ctrls = m_func.getControlPoints();
	if (ctrls.size() >= 3) {
		glBegin(GL_LINE_STRIP);
		glColor3d(0.0, 0.0, 0.0);
		for (double x = 0; x <= 1; x += k_evalStep) {
			//operator() of m_func takes the scale into account, but we don't want scaled y values here
			double y = m_func(x) / m_yScale;
			glVertex3d(x, y, 0);
		}
		glEnd();
	}
	
	// highlight the control points
	// using m_x and m_y
	
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor3d(1.0, 0.0, 0.0);
	for (auto [x, y] : ctrls) {
		glVertex3d(x, y, 0);
	}
	glEnd();
}

void kfcw::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glShadeModel(GL_FLAT);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);
	glPointSize(7.0f);
}

void kfcw::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadMatrixd(glm::value_ptr(getProjection()));
	drawGrid();
	drawSpline();
}

kfcw::m4 kfcw::getProjection() const {
	return glm::ortho(m_viewMin.x, m_viewMax.x, m_viewMin.y, m_viewMax.y, -1.0, 1.0);
}

void kfcw::resizeGL(int width, int height) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixd(glm::value_ptr(getProjection()));
}

kfcw::v2 kfcw::screen2world(iv2 screenPos) const {
	double w = this->width(), h = this->height();
	v2 ndcCoords { 2 * screenPos.x / w - 1, 1 - 2 * screenPos.y / h };
	v4 world = glm::inverse(getProjection()) * v4 { ndcCoords, 0, 1 };
	world /= world.w;

	return v2 { world.x, world.y };
}

kfcw::iv2 kfcw::world2screen(v2 world) const {
	v4 clipCoords = getProjection() * v4 { world, 0.0, 1.0 };
	v3 ndcCoords = v3 { clipCoords } / clipCoords.w;
	
	return iv2 {
		(ndcCoords.x + 1.0) * 0.5 * this->width(),
		(1.0 - ndcCoords.y) * 0.5 * this->height()
	};
}

bool verifyPos(double x, double y) {
	return x >= 0 && x <= 1 && y >= 0 && y <= 1;
}

void kfcw::mousePressEvent(QMouseEvent* event) {
	v2 pos = screen2world({ event->x(), event->y() });

	if (event->buttons() == Qt::LeftButton || event->button() == Qt::RightButton) {
		auto& ctrls = m_func.getControlPoints();

		if (!verifyPos(pos.x, pos.y)) {
			return;
		}
		for (size_t i = 1; i < ctrls.size(); ++i) {
			auto [x, y] = ctrls[i];
			auto [px, py] = ctrls[i - 1];
			if (i < ctrls.size() - 1) {
				auto [nx, ny] = ctrls[i + 1];

				if (std::abs(x - pos.x) <= k_editTolerance &&
					std::abs(y - pos.y) <= k_editTolerance) {
					m_curEdit = PointEditData{
						ctrls.cbegin() + i,
						px + 0.01,
						nx - 0.01
					};
					return;
				}
			}

			if(px < pos.x && x > pos.x) {
				m_func.insert(ctrls.cbegin() + i, pos.x, pos.y);
				update();
				return;
			}
		}
	} else if (event->buttons() == Qt::MiddleButton) {
		m_lastPos = pos;
	}
}

void kfcw::editPoint(QMouseEvent* event) {
	if (m_curEdit) {
		v2 pos = screen2world({ event->x(), event->y() });
		pos.x = glm::clamp(pos.x, m_curEdit->xMin, m_curEdit->xMax);
		pos.y = glm::clamp(pos.y, 0.0, 1.0);

		if (!verifyPos(pos.x, pos.y)) {
			m_curEdit = std::nullopt;
			return;
		}
		m_func.setControlPoint(m_curEdit->it, pos.x, pos.y);
		update();
	}
}

void kfcw::mouseReleaseEvent(QMouseEvent* event) {
	if (event->buttons() == Qt::LeftButton || event->button() == Qt::RightButton) {
		if (m_curEdit) {
			m_curEdit = std::nullopt;
		}

		emit curveChanged();
	}
}

void kfcw::mouseMoveEvent(QMouseEvent* event) {
	v2 pos = screen2world({ event->x(), event->y() });
	v2 delta = m_lastPos - pos;

	if (event->buttons() & Qt::MiddleButton) {
		v2 viewSize = m_viewMax - m_viewMin;

		m_viewMin += delta * 0.5;
		m_viewMax += delta * 0.5;

		m_viewMin = glm::clamp(m_viewMin, {0, 0}, v2{1, 1} - viewSize);
		m_viewMax = glm::clamp(m_viewMax, viewSize, {1, 1});

		update();
	} else if (event->buttons() == Qt::LeftButton || event->button() == Qt::RightButton) {
		editPoint(event);
	}
	m_lastPos = pos;
}

void kfcw::wheelEvent(QWheelEvent* e)
{
	double factor = e->angleDelta().y() > 0 ? 1 / k_zoomFactor : k_zoomFactor;
	m_viewMax *= factor;
	// make sure it's not too crazy
	m_viewMax = glm::clamp(m_viewMax, { 0.3, 0.3 }, { 1.0, 1.0 });

	update();
}