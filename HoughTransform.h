#ifndef HOUGHTRANSFORM_H
#define HOUGHTRANSFORM_H

class HoughTransform
{
public:
    HoughTransform() { }
    HoughTransform(double arg_of_max_mod, double value_of_max_mod, double radius_step = 0.1);
    ~HoughTransform();

    void Init(double arg_of_max_abs, double value_of_max_abs, double radius_step = 0.1);

    double GetMaxRadius() const { return _max_radius; }
    double GetRadiusStep() const { return _radius_step; }

    void AddPoint(double x, double y, double weight = 1);

    double GetNormalAngleInDegr();
    double GetNormalRadius();

    bool IsPointFromRecogLine(double x, double y);
    double GetLineAngleCoef();
    double GetLineShift();

    void Clear();

    //Для тестов
    int ColumnsCnt() const { return _columns_as_radius_values; }

private:
    void CreateTable();
    void DeleteTable();

    void FindResult();

    bool _is_result_found = false;
    const double _MIN_RADIUS_STEP = 0.1, _MAX_RADIUS_STEP = 1;
    int _row_of_max, _col_of_max;
    double _res_angle_in_degr, _res_radius;

    double **_matrix = nullptr;
    const int _ROWS_AS_ANGLE_VALUES = 360;
    int _columns_as_radius_values;
    double _radius_step, _max_radius;
};

#endif // HOUGHTRANSFORM_H
