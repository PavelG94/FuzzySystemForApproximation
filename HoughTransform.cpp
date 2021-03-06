#include <cassert>

#include <QtMath>
#include "HoughTransform.h"

HoughTransform::HoughTransform(double arg_of_max_mod, double value_of_max_mod, double radius_step)
{
    Init(arg_of_max_mod, value_of_max_mod, radius_step);
}

HoughTransform::~HoughTransform()
{
    DeleteTable();
}

void HoughTransform::Init(double arg_of_max_abs, double value_of_max_abs, double radius_step)
{
    double arg_sqr = arg_of_max_abs * arg_of_max_abs;
    double value_sqr = value_of_max_abs * value_of_max_abs;
    radius_step = qMax(_MIN_RADIUS_STEP, qMin(radius_step, _MAX_RADIUS_STEP));

    this->_radius_step = radius_step;
    _max_radius = qCeil(qSqrt(arg_sqr + value_sqr));
    //Один столбец для нуля
    _columns_as_radius_values = qRound(_max_radius / radius_step) + 1;

    CreateTable();
    Clear();
}

void HoughTransform::AddPoint(double x, double y, double weight)
{
    assert(_matrix != nullptr);
    for (int angle_in_degr = 0; angle_in_degr < _ROWS_AS_ANGLE_VALUES; angle_in_degr++) {
        double angle_in_rad = qDegreesToRadians(double(angle_in_degr));
        double radius = x*qCos(angle_in_rad) + y*qSin(angle_in_rad);
        if (0 <= radius && radius <= _max_radius) {
            int row = angle_in_degr;
            int col = qRound(radius / _radius_step);
            _matrix[row][col] += weight;
        }
    }
    _is_result_found = false;
}

double HoughTransform::GetNormalAngleInDegr()
{
    if (_is_result_found == false) FindResult();
    return _res_angle_in_degr;
}

double HoughTransform::GetNormalRadius()
{
    if (_is_result_found == false) FindResult();
    return _res_radius;
}

bool HoughTransform::IsPointFromRecogLine(double x, double y)
{
    if (_is_result_found == false) FindResult();

    for (int angle_in_degr = 0; angle_in_degr < _ROWS_AS_ANGLE_VALUES; angle_in_degr++) {
        double angle_in_rad = qDegreesToRadians(double(angle_in_degr));
        double radius = x*qCos(angle_in_rad) + y*qSin(angle_in_rad);
        if (0 <= radius && radius <= _max_radius) {
            int row = angle_in_degr;
            int col = qRound(radius / _radius_step);
            if (row == _row_of_max && col == _col_of_max) {
                return true;
           }
        }
    }
    return false;
}

double HoughTransform::GetLineAngleCoef()
{
    if (_is_result_found == false) FindResult();
    // Формула прямой: x*cos(alpha) + y*sin(alpha) = r
    double angle_in_rad = qDegreesToRadians(_res_angle_in_degr);
    double angle_coef = -qCos(angle_in_rad)/qSin(angle_in_rad);
    return angle_coef;
}

double HoughTransform::GetLineShift()
{
    if (_is_result_found == false) FindResult();
    // Формула прямой: x*cos(alpha) + y*sin(alpha) = r
    double angle_in_rad = qDegreesToRadians(_res_angle_in_degr);
    double line_shift = _res_radius/qSin(angle_in_rad);
    return line_shift;
}

void HoughTransform::CreateTable()
{
    _matrix = new double*[_ROWS_AS_ANGLE_VALUES];
    for (int i = 0; i < _ROWS_AS_ANGLE_VALUES; ++i) {
       _matrix[i] = new double[_columns_as_radius_values];
    }
}

void HoughTransform::Clear()
{
    for (int row = 0; row < _ROWS_AS_ANGLE_VALUES; ++row) {
        for (int col = 0; col < _columns_as_radius_values; ++col) {
            _matrix[row][col] = 0;
        }
    }
}

void HoughTransform::DeleteTable()
{
    for (int i = 0; i < _ROWS_AS_ANGLE_VALUES; ++i) {
       delete []_matrix[i];
    }
    delete []_matrix;
}

void HoughTransform::FindResult()
{
    _row_of_max = 0;
    _col_of_max = 0;
    for (int row = 0; row < _ROWS_AS_ANGLE_VALUES; ++row) {
        for (int col = 0; col < _columns_as_radius_values; ++col) {
            if (_matrix[_row_of_max][_col_of_max] < _matrix[row][col]) {
                _row_of_max = row;
                _col_of_max = col;
            }
        }
    }
    _res_angle_in_degr = _row_of_max; _res_radius = _col_of_max * _radius_step;
    _is_result_found = true;
}
