#include <cassert>
#include <QtMath>
#include <QVector>
#include "MLS.h"
#include "CntlBuilder.h"

double CntlBuilder::CalcSumError(SugenoCntl &cntl, const QMap<double, double> &points)
{
    double sum_error(0);
    QMapIterator<double,double> it(points);
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        double y_cntl = cntl(x);
        sum_error += (y - y_cntl)*(y - y_cntl);
    }
    return sum_error;
}

CntlBuilder::CntlBuilder()
{
}

void CntlBuilder::SetData(SugenoCntl *cntl, double max_abs_arg, double max_abs_value)
{
    assert(0 <= max_abs_arg && 0 <= max_abs_value);
    _cntl = cntl;
    _hough.Init(max_abs_arg, max_abs_value);
    _is_data_set = true;
}

void CntlBuilder::SetInput(const QMap<double, double> &in_points)
{
    //! флаг установки данных
    assert(MIN_POINTS_FOR_LINE_DEF <= in_points.size());
    _in_points = in_points;
    const double init_error = 1;
    QMapIterator<double,double> it(_in_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key();
        _errors.insert(x, init_error);
    }
    _max_error = 1;
}

bool CntlBuilder::BuildStep()
{
    UseErrorsForRecog();

    //Условие 1 остановки обучения
    if (_max_error <= MAX_ERROR_EPS) return false;

    FindPointsWithSmallDistToRecogLine(_in_points);

    //Условие 2 остановки обучения
    if (_small_dist_to_recog_line_points.size() < MIN_POINTS_FOR_LINE_DEF)
        return false;

    //Получение уточнённых с помощью МНК параметров распознанной прямой
    QVector<double> x_vals(_small_dist_to_recog_line_points.size());
    QVector<double> y_vals(_small_dist_to_recog_line_points.size());
    QMapIterator<double,double> it(_small_dist_to_recog_line_points);
    int curr_vec_id = 0;
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        x_vals[curr_vec_id] = x;
        y_vals[curr_vec_id] = y;
        ++curr_vec_id;
    }
    double a = 0, b = 0;
    ApplyMLS(a, b, x_vals, y_vals);
    _angle_coef = a;
    _line_shift = b;

    //Функция нормального распределения сделает контроллер всюду определённым
    UnaryFunc m_func = SugenoCntl::GenNormalFunc(x_vals);

    //Добвление нового правила
    UnaryFunc linear_func = std::function<double(double)>([a,b](double x)->double { return a*x + b; });
    _cntl->AddRule(m_func, linear_func);

    //Пересчёт значений входа
    RecalcInPointsValues();

    return true;
}

QMap<double, double> CntlBuilder::CalcErrors(const QMap<double, double> &points)
{
    QMapIterator<double,double> points_it(points);
    _max_error = 0;
    while(points_it.hasNext()) {
        points_it.next();
        double y = points_it.value();
        if (qAbs(y) > _max_error) _max_error = qAbs(y);
    }
    if (_max_error <= MAX_ERROR_EPS) return QMap<double,double>();

    QMap<double,double> errors;
    points_it.toFront();
    while (points_it.hasNext()) {
        points_it.next();
        double x = points_it.key(), error = qAbs(points_it.value());
        double n_error = error/_max_error;
        errors.insert(x, n_error);
    }
    return errors;
}

void CntlBuilder::CalcSmallCntlErrorPoints(const QMap<double,double> &points)
{
    _small_error_points.clear();
    QMapIterator<double,double> it(points);
    SugenoCntl &cntl = *_cntl;
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        double cntl_val = cntl(x);
        if (qAbs(y - cntl_val) <= CNTL_ERROR_EPS) {
            _small_error_points.insert(x, y);
        }
    }
}

void CntlBuilder::UseErrorsForRecog()
{
    _hough.Clear();
    if (0 < _cntl->RulesCnt()) {
        _errors = CalcErrors(_in_points);
    }
    QMapIterator<double,double> it(_errors);
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = _in_points.value(x), error = it.value();
        _hough.AddError(x,y,error);
    }
}

double CntlBuilder::CalcDistToRecogLine(double normal_angle_in_rad, double normal_length, double x, double y)
{
   //Формула для вычисления расстояния от точки до прямой
   double dist = qAbs( normal_length - ( qCos(normal_angle_in_rad)*x + qSin(normal_angle_in_rad)*y ) );
   return dist;
}

void CntlBuilder::FindPointsWithSmallDistToRecogLine(const QMap<double, double> &points)
{
    _small_dist_to_recog_line_points = _hough.GetPointsFromRecogLine(points);
}

void CntlBuilder::RecalcInPointsValues()
{
    assert(_cntl != nullptr);
    SugenoCntl &cntl = *_cntl;
    QMutableMapIterator<double,double> it(_in_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        double new_y = y - cntl(x);
        it.value() = new_y;
    }
}
