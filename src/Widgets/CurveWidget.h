#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory>
#include "../Phyllotaxis/UserCurveLenFunction.h"

class CurveWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    static constexpr double s_step = 0.1;
    explicit CurveWidget(QWidget* parent = nullptr);
    ~CurveWidget() override;
    void setCurve(std::shared_ptr<UserCurveLenFunction> const& curveFunc);
    bool valid() const;
    auto getYMin() const -> double;
    auto getYMax() const -> double;
    auto getViewYMin() const -> double;
    auto getViewYMax() const -> double;

signals:
    void curveUpdated();

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    std::weak_ptr<UserCurveLenFunction> m_pfunc;
    double m_aspectRatio;
    double m_minY, m_maxY;
	std::vector<std::pair<double, double>> m_verts;
};