#include "CurveWidget.h"

CurveWidget::CurveWidget(QWidget* parent) : QOpenGLWidget(parent) {}
CurveWidget::~CurveWidget() {}

void CurveWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    m_aspectRatio = width() / static_cast<double>(height());
    m_verts.reserve(static_cast<int>(m_aspectRatio / s_step));
}

void CurveWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_aspectRatio, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3d(0, 1, 0);
    glBegin(GL_LINES);
    for(auto&& v : m_verts) {
        glVertex3d(v.first, v.second, 0);
    }
    glEnd();
}

void CurveWidget::setCurve(std::shared_ptr<UserCurveLenFunction> const& curveFunc) {
    m_pfunc = curveFunc;
    if (!valid()) {
        m_verts.clear();
    } else {
        double minY = std::numeric_limits<double>::max();
    	double maxY = std::numeric_limits<double>::lowest();

        for (double x = 0; x < 1.0; x += s_step) {
            double y = (*m_pfunc.lock())(x);
            maxY = std::max(maxY, y);
            minY = std::min(minY, y);
            m_verts.emplace_back(x, y); 
        }

        for(auto&& p : m_verts) {
            // x : [0, aspect]
            // y : [0, 1]

            p.first *= m_aspectRatio;
            if (minY < 0) {
                p.second += minY;
            }
            if (std::abs(maxY) > std::numeric_limits<double>::epsilon()) {
                p.second /= maxY;
            }
        }
    }
}

bool CurveWidget::valid() const {
    if (m_pfunc.lock()) {
        return *m_pfunc.lock();
    } else {
        return false;
    }
}