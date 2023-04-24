#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QtGui/QMouseEvent>
#include <QVector>

#include <glm/glm.hpp>
#include "WidgetDataSaver.h"
#include "../CurveLenFunction/KeyframeCurveLenFunction.h"

class KeyframeCurveWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

private:
    using iv2 = glm::ivec2;
    using v2 = glm::dvec2;
    using v3 = glm::dvec3;
    using v4 = glm::dvec4;
    using m4 = glm::dmat4;

public:
    using SplineType = KeyframeCurveLenFunction::SplineType;

	explicit KeyframeCurveWidget(QWidget* parent = nullptr, SplineType type = SplineType::Linear);

	void setYScale(double scale);
    void setSplineType(SplineType type);
    void setControlPoints(ControlPointArray::ConstIterator first, ControlPointArray::ConstIterator last);

	[[nondiscard]] auto getFunction() const {
        return std::make_shared<KeyframeCurveLenFunction>(m_func);
    }

signals:
    void curveChanged();

private:
    void updateCurve();
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

private:
    double m_yScale;
    v2 m_viewMin, m_viewMax;
    v2 m_lastPos;
    KeyframeCurveLenFunction m_func;
    struct PointEditData {
        ControlPointArray::ConstIterator it;
        double xMin, xMax;
    };
    std::optional<PointEditData> m_curEdit;
};