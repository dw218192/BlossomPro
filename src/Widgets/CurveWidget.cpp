#include "CurveWidget.h"

CurveWidget::CurveWidget(QWidget* parent) : QOpenGLWidget(parent), m_aspectRatio(1), m_minY(0), m_maxY(1) {}
CurveWidget::~CurveWidget() = default;

void CurveWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    m_aspectRatio = width() / static_cast<double>(height());
    m_verts.reserve(static_cast<int>(m_aspectRatio / s_step));
}

auto CurveWidget::getYMin() const -> double {
    return m_minY;
}
auto CurveWidget::getYMax() const -> double {
    return m_maxY;
}
auto CurveWidget::getViewYMin() const -> double {
    return m_minY - 1;
}
auto CurveWidget::getViewYMax() const -> double {
    return m_maxY + 1;
}
void CurveWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_aspectRatio, getViewYMin(), getViewYMax(), -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3d(0, 1, 0);
    glBegin(GL_LINE_STRIP);
    for(auto [x, y] : m_verts) {
        glVertex3d(x, y, 0);
    }
    glEnd();
}

void CurveWidget::setCurve(std::shared_ptr<UserCurveLenFunction> const& curveFunc) {
    m_pfunc = curveFunc;
    m_verts.clear();
    m_minY = 0;
    m_maxY = 1;

    if (valid()) {
        auto pfunc = m_pfunc.lock();
        double minY = std::numeric_limits<double>::max();
    	double maxY = std::numeric_limits<double>::lowest();

        for (double x = 0; x <= 1.0; x += s_step) {
            double y = (*pfunc)(x);
            maxY = std::max(maxY, y);
            minY = std::min(minY, y);

            // convert x to be in the range [0, aspect]
            m_verts.emplace_back(x * m_aspectRatio, y);
        }

        m_minY = minY;
        m_maxY = maxY;
    }

    // repaint
    update();
    emit curveUpdated();
}

bool CurveWidget::valid() const {
    if (auto const ptr = m_pfunc.lock()) {
        return ptr->valid();
    } else {
        return false;
    }
}