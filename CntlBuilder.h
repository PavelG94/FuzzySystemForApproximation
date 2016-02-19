#ifndef CNTLBUILDER_H
#define CNTLBUILDER_H

#include <QMap>

#include "UnaryFunc.h"
#include "HoughTransform.h"
#include "SugenoCntl.h"

class CntlBuilder
{
public:
    const int MIN_POINTS_FOR_LINE_DEF = 2;
    const double DIST_EPS = 0.1;
    const double CNTL_ERROR_EPS = 0.5;

    static double CalcSumError(SugenoCntl &cntl, const QMap<double,double> &points);

    CntlBuilder();
    void SetData(SugenoCntl *cntl, double max_abs_arg, double max_abs_value);

    //Возвращает true, если было добавлено новое правило
    bool BuildStep(const QMap<double,double> &rest_points);

    QMap<double,double> GetSmallErrorPoints() const { return _small_error_points; }
    double GetAngleCoefOfRecogLine() const { return _angle_coef; }
    double GetShiftOfRecogLine() const { return _line_shift; }

private:
    void UsePointsForRecog(const QMap<double, double> &points);
    double CalcDistToRecogLine(double normal_angle_in_rad, double normal_length, double x, double y);
    void FindPointsWithSmallDistToRecogLine(const QMap<double,double> &points);
    void CalcSmallCntlErrorPoints(const QMap<double,double> &points);

private:
    bool _is_data_set = false;
    SugenoCntl *_cntl = nullptr;
    HoughTransform _hough;
    double _angle_coef = 0, _line_shift = 0;
    QMap<double,double> _small_dist_to_recog_line_points;
    QMap<double,double> _small_error_points;
};

#endif // CNTLBUILDER_H
