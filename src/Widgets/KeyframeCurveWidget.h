#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QtGui/QMouseEvent>
#include <vector>
#include <glm/glm.hpp>

#include "aaCurve.h"

class KeyframeCurveWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

private:
    glm::vec2 m_viewMax;
    glm::vec2 m_camPos;
    glm::vec2 m_lastPos;

    glm::vec2 m_camViewScale;



    int ctrlSelected;

    std::vector<double>* m_framedata;
    bool m_bIfModify;

    aaAaa::aaSpline* m_spline_data;
    double m_deltaT;
public:
    explicit KeyframeCurveWidget(QWidget* parent = 0);

public:
    void setAE(aaAaa::aaSpline* sdata);

    void setCurrentSelected(int index);

private:
    void renderText(glm::ivec2 screenPos, QString text);
    void drawGrid();
    void drawSpline();

    glm::vec2 screen2world(glm::ivec2 screenPos);
    glm::ivec2 world2screen(glm::vec2 world);

    int checkSelected(aaAaa::Vector2 point);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent*) override;

signals:
    void selectValuesChanged(float t, float v);
};