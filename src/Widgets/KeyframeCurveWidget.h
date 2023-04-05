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
    int m_axis;
    glm::vec3 m_CameraPos;

    static const int Z_VALUE = -10;
    static const int CAMERA_D = 1;
    static const int Y_FACTOR = 1;
    static const float MY_PI;

    QPoint lastPos;

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
    void renderText(GLdouble x, GLdouble y, GLdouble z, QString text);
    void drawGrid();
    void drawSpline();

    aaAaa::Vector2 screen2gl(int x, int y);
    int checkSelected(aaAaa::Vector2 point);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    //void mousePressEvent(QMouseEvent* event) override;
    //void mouseReleaseEvent(QMouseEvent* event) override;
    //void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent*) override;

signals:
    void selectValuesChanged(float t, float v);
};