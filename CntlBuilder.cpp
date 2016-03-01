#include <cassert>
#include <QtMath>
#include <QVector>
#include "MLS.h"
#include "CntlBuilder.h"

void CntlBuilder::SetData(UnaryFunc &f, double x_min, double x_max, double step)
{
    assert(0 < step);
    int points_cnt = (x_max - x_min + step) / step; // [x_min, x_max]
    assert(1 < points_cnt);

    double x_of_max_abs = x_min, max_abs_y = qAbs(f(x_min));
    for (int i = 0; i < points_cnt; ++i) {
        double x = x_min + i*step;
        double y  = f(x);
        if (f.IsLastResValid() == true) {
            //вставка без повторов
            _input_points.insert(x,y);
            if (max_abs_y < qAbs(y)) {
                x_of_max_abs = x;
                max_abs_y = qAbs(y);
            }
        }
    }
    InitErrorsVector(_input_points);
    PrepareToLearning(x_of_max_abs, max_abs_y);
    _is_ready_to_build = true;
}

void CntlBuilder::SetData(const QVector<double> &x_vals, const QVector<double> &y_vals)
{
    //Более общий случай, можно подать на вход последовательность точек, содержащих шум
    assert(1 < x_vals.size());
    assert(y_vals.size() == x_vals.size());

    double x_of_max_abs = x_vals[0], max_abs_y = qAbs(y_vals[0]);
    for (int i = 0; i < x_vals.size(); ++i) {
        double x = x_vals[i], y = y_vals[i];
        //вставка без повторов
        _input_points.insert(x,y);
        if (max_abs_y < qAbs(y)) {
            x_of_max_abs = x;
            max_abs_y = qAbs(y);
        }
    }
    InitErrorsVector(_input_points);
    PrepareToLearning(x_of_max_abs, max_abs_y);
    _is_ready_to_build = true;
}

double CntlBuilder::CalcSumError()
{
    double sum_error(0);
    QMapIterator<double,double> it(_input_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        double y_cntl = _cntl(x);
        sum_error += (y - y_cntl)*(y - y_cntl);
    }
    return sum_error;
}

bool CntlBuilder::BuildStep()
{
    if (_is_ready_to_build == false) return false;
    if (MAX_LEARNING_STEPS <= _steps_done) return false; //Условие остановки обучения

    RecalcErrors();
    if (_max_error <= MAX_ERROR_EPS) return false;  //Условие остановки обучения

    RecogNextLine();
    _recog_line_points = _hough.GetPointsFromRecogLine(_input_points);
    if (_recog_line_points.size() < MIN_POINTS_FOR_LINE_DEF) return false; //Условие остановки обучения

    CalcClarifiedRecogLineParams(_recog_line_points);
    AddNewRule(_recog_line_points, _recog_line_angle_coef, _recog_line_shift);
    ++_steps_done;
    return true;
}

void CntlBuilder::Build()
{
    bool is_done = true;
    while (is_done == true) {
        is_done = BuildStep();
    }
}

void CntlBuilder::InitErrorsVector(const QMap<double,double> &points)
{
    _errors.clear();
    QMapIterator<double,double> points_it(points);
    const double init_error = 1;
    while (points_it.hasNext()) {
        points_it.next();
        double x = points_it.key();
        _errors.insert(x,init_error);
    }
    _max_error = init_error;
}

void CntlBuilder::PrepareToLearning(double x_of_max_abs_y, double max_abs_y)
{
    assert(0 <= x_of_max_abs_y && 0 <= max_abs_y);
    _cntl.Clear();
    _hough.Init(x_of_max_abs_y, max_abs_y);
    _steps_done = 0;
}

void CntlBuilder::RecalcErrors()
{
    if (_cntl.RulesCnt() < 1) {
        return;
    }

    _max_error = 0;
    QMapIterator<double,double> points_it(_input_points);
    while(points_it.hasNext()) {
        points_it.next();
        double x = points_it.key(), y = points_it.value();
        double y_cntl = _cntl(x);
        double error = qAbs(y - y_cntl);
        _errors[x] = error;
        if (error > _max_error) _max_error = error;
    }

    if (_max_error == 0) return;

    //Нормировка
    QMutableMapIterator<double,double> errors_it(_errors);
    while (errors_it.hasNext()) {
        errors_it.next();
        double error = errors_it.value();
        double n_error = error/_max_error;
        errors_it.value() = n_error;
    }
}

void CntlBuilder::RecogNextLine()
{
    _hough.Clear();
    QMapIterator<double,double> errors_it(_errors);
    while (errors_it.hasNext()) {
        errors_it.next();
        double x = errors_it.key(), y = _input_points.value(x), error = errors_it.value();
        _hough.AddError(x,y,error);
    }
}

void CntlBuilder::CalcClarifiedRecogLineParams(const QMap<double, double> &recog_line_points)
{
    QVector<double> x_vals(recog_line_points.size());
    QVector<double> y_vals(recog_line_points.size());
    QMapIterator<double,double> it(recog_line_points);
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
    _recog_line_angle_coef = a;
    _recog_line_shift = b;
}

void CntlBuilder::AddNewRule(const QMap<double, double> &recog_line_points, double recog_line_angle_coef, double recog_line_shift)
{
    QVector<double> x_vals = recog_line_points.keys().toVector();
    //Функция нормального распределения, контроллер получается всюду определённым
    UnaryFunc m_func = SugenoCntl::GenNormalFunc(x_vals);

    double a = recog_line_angle_coef, b = recog_line_shift;
    UnaryFunc conseq_func = std::function<double(double)>([a,b](double x)->double { return a*x + b; });
    _cntl.AddRule(m_func, conseq_func);
}
