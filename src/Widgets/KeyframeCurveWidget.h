#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QtGui/QMouseEvent>
#include <glm/glm.hpp>
#include "spline.h"

class KeyframeCurveWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit KeyframeCurveWidget(QWidget* parent = 0);

private:
    using iv2 = glm::ivec2;
    using v2 = glm::dvec2;
    using v3 = glm::dvec3;
    using v4 = glm::dvec4;
    using m4 = glm::dmat4;

    m4 getProjection() const;
    void renderText(QPainter& painter, glm::ivec2 screenPos, QString text);
    void drawGrid();
    void drawSpline();
    v2 screen2world(iv2 screenPos) const;
    iv2 world2screen(v2 world) const;
    void editPoint(QMouseEvent* event);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent*) override;

// signals: 

private:
    v2 m_viewMin, m_viewMax;
    v2 m_lastPos;

    std::vector<double> m_x, m_y;
    tk::spline m_spline;

    struct PointEditData {
        size_t index;
        double xMin, xMax;
    };
    std::optional<PointEditData> m_curEdit;
};