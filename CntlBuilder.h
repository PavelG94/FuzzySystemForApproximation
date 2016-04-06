#ifndef CNTLBUILDER_H
#define CNTLBUILDER_H

#include <QVector>

#include "UnaryFunc.h"
#include "HoughTransform.h"
#include "SugenoCntl.h"

class CntlBuilder
{
public:
    enum DistCluster { dcSHORT, dcLONG };

    const int MAX_LEARNING_STEPS = 100;
    const int MIN_POINTS_FOR_LINE_DEF = 2;

    void SetData(UnaryFunc &f, double x_min, double x_max, double step);
    void SetData(const QVector<double> &x_vals, const QVector<double> &y_vals);

    double CalcSumError();
    void GetInputPointsX(QVector<double> &x_vals) const;
    void GetInputPointsY(QVector<double> &y_vals) const;
    void GetRestInputPointsX(QVector<double> &x_vals) const;
    void GetRestInputPointsY(QVector<double> &y_vals) const;

    void GetRecogLinePoints(QVector<double> &x_vals, QVector<double> &y_vals) const;
    double GetRecogLineAngleCoef() const { return _recog_line_angle_coef; }
    double GetRecogLineShift() const { return _recog_line_shift; }
    SugenoCntl& GetController() { return _cntl; }

    bool BuildStep(); //Возвращает true, если было добавлено новое правило
    void Build();

protected:
    struct PointInfo
    {
        PointInfo(double x = 0, double y = 0, bool is_removed = false)
            : x(x), y(y), is_removed(is_removed) { }
        double x;
        double y;
        bool is_removed;
    };
    QVector<DistCluster> KMeansByDist();

    void AscSortPointsByX(QVector<PointInfo> &points);
    void PrepareToLearning(double x_of_max_abs_y, double max_abs_y);
    void RecogNextLine();
    void PickPointsFromRecogLine();

    void FilterRecogLinePoints();
    void ClarifyRecogLineParamsViaMLS();

    void AddNewRule();
    void MarkPointsFromRecogLineAsRemoved();

protected:
    QVector<PointInfo> _input_points;
    int _not_removed_points_cnt = 0;
    QVector<PointInfo*> _recog_line_points_ptrs;
    double _recog_line_angle_coef = 0, _recog_line_shift = 0;
    int _steps_done = 0;
    bool _is_ready_to_build = false;

    HoughTransform _hough;
    SugenoCntl _cntl;
};

#endif // CNTLBUILDER_H
