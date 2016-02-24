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

bool CntlBuilder::BuildStep(const QMap<double, double> &rest_points)
{
    if (rest_points.size() < MIN_POINTS_FOR_LINE_DEF) return false;
    UsePointsForRecog(rest_points);

    //Условие 1 остановки обучения
    if (_max_error <= MAX_ERROR_EPS) return false;

    FindPointsWithSmallDistToRecogLine(rest_points);

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

    return true;
}

QMap<double, double> CntlBuilder::CalcErrors(const QMap<double, double> &points)
{
    assert(_cntl != nullptr);
    SugenoCntl cntl = *(_cntl);
    QMap<double,double> errors;
    QMapIterator<double,double> points_it(points);
    _max_error = 0;
    while(points_it.hasNext()) {
        points_it.next();
        double x = points_it.key(), y = points_it.value();
        double y_cntl = cntl(x);
        double error = qAbs(y - y_cntl);
        errors.insert(x, error);
        if (error > _max_error) _max_error = error;
    }
    if (_max_error <= MAX_ERROR_EPS) return QMap<double,double>();

    //Нормировка
    QMutableMapIterator<double,double> errors_it(errors);
    while (errors_it.hasNext()) {
        errors_it.next();
        double error = errors_it.value();
        double n_error = error/_max_error;
        errors_it.value() = n_error;
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

void CntlBuilder::UsePointsForRecog(const QMap<double, double> &points)
{
    _hough.Clear();
    if (points.isEmpty()) return;
    if (_cntl->RulesCnt() == 0) {
        QMapIterator<double,double> it(points);
        while (it.hasNext()) {
            it.next();
            double x = it.key(), y = it.value();
            _hough.AddPoint(x, y);
        }
        _max_error = 1;
    } else {
        //Вычисление нормированных ошибок, т.е. диапазон их зн-ий = [0,1]
        QMap<double,double> errors = CalcErrors(points);
        QMapIterator<double,double> it(errors);
        while (it.hasNext()) {
            it.next();
            double x = it.key(), y = points.value(x), error = it.value();
            _hough.AddError(x,y,error);
        }
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
