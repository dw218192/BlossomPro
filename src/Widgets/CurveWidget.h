#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory>
#include "../Phyllotaxis/UserCurveLenFunction.h"

struct CurveWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    static constexpr double s_step = 0.1;
    explicit CurveWidget(QWidget* parent = nullptr);
    ~CurveWidget() override;
    void setCurve(std::shared_ptr<UserCurveLenFunction> const& curveFunc);
    bool valid() const;

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    std::weak_ptr<UserCurveLenFunction> m_pfunc;
    double m_aspectRatio;
    std::vector<std::pair<double, double>> m_verts;
};