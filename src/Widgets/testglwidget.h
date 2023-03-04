#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

class TestGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    explicit TestGLWidget(QWidget* parent = nullptr) : QOpenGLWidget(parent) {}

    void initializeGL() override {
        initializeOpenGLFunctions();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glShadeModel(GL_SMOOTH);
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        int const w = width();
        int const h = height();
        double const aspect = static_cast<double>(w) / h;
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_STRIP);
        for (double t = 0.0; t <= 1.0; t += deltaT) {
            glVertex2d(t * 2.0 - 1.0, t * 2.0 - 1.0);
        }
        glEnd();
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
    }

private:
    const double deltaT = 0.1;
};