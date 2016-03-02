#ifndef CNTLBUILDER_H
#define CNTLBUILDER_H

#include <QMap>

#include "UnaryFunc.h"
#include "HoughTransform.h"
#include "SugenoCntl.h"

class CntlBuilder
{
public:
    const int MAX_LEARNING_STEPS = 100;
    const int MIN_POINTS_FOR_LINE_DEF = 2;
    const double MAX_ERROR_EPS = 0.1;

    void SetData(UnaryFunc &f, double x_min, double x_max, double step);
    void SetData(const QVector<double> &x_vals, const QVector<double> &y_vals);

    double CalcSumError();
    QMap<double,double> GetInputPoints() const { return _input_points; }
    QMap<double,double> GetRecogLinePoints() const { return _recog_line_points; }
    double GetRecogLineAngleCoef() const { return _recog_line_angle_coef; }
    double GetRecogLineShift() const { return _recog_line_shift; }
    SugenoCntl& GetController() { return _cntl; }

    bool BuildStep(); //Возвращает true, если было добавлено новое правило
    void Build();

private:
    void InitErrorsVector(const QMap<double, double> &points);
    void PrepareToLearning(double x_of_max_abs_y, double max_abs_y);

    void RecalcErrors();
    void RecogNextLine();
    void CalcClarifiedRecogLineParams(const QMap<double,double> &recog_line_points);
    void AddNewRule(const QMap<double,double> &recog_line_points, double recog_line_angle_coef, double recog_line_shift);

private:
    QMap<double,double> _input_points;
    QMap<double,double> _errors;
    QMap<double,double> _recog_line_points;
    double _recog_line_angle_coef = 0, _recog_line_shift = 0;
    double _max_error = -1;
    int _steps_done = 0;
    bool _is_ready_to_build = false;

    HoughTransform _hough;
    SugenoCntl _cntl;
};

#endif // CNTLBUILDER_H
