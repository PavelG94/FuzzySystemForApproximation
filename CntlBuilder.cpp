#include <cassert>
#include <algorithm>

#include <QDebug>
#include <QtMath>

#include <armadillo>

#include "CntlBuilder.h"

void CntlBuilder::SetData(UnaryFunc &f, double x_min, double x_max, double step)
{
    assert(0 < step);
    int points_cnt = (x_max - x_min + step) / step; // [x_min, x_max]
    assert(1 < points_cnt);

    double x_of_max_abs = x_min, max_abs_y = qAbs( f(x_min) );
    _input_points.resize(points_cnt);
    for (int i = 0; i < points_cnt; ++i) {
        double x = x_min + i*step;
        double y = f(x);
        y = (f.IsLastResValid() == true)? y: 0;
        //повторы во входной последовательности значений не отслеживаются!
        _input_points[i] = PointInfo(x,y,false);
        if (max_abs_y < qAbs(y)) {
            x_of_max_abs = x;
            max_abs_y = qAbs(y);
        }
    }

    PrepareToLearning(x_of_max_abs, max_abs_y);
}

void CntlBuilder::SetData(const QVector<double> &x_vals, const QVector<double> &y_vals)
{
    //Более общий случай, можно подать на вход последовательность точек, содержащих шум
    assert(1 < x_vals.size());
    assert(y_vals.size() == x_vals.size());

    double x_of_max_abs = x_vals[0], max_abs_y = qAbs( y_vals[0] );
    _input_points.resize(x_vals.size());
    for (int i = 0; i < x_vals.size(); ++i) {
        double x = x_vals[i], y = y_vals[i];
        //повторы во входной последовательности значений не отслеживаются!
        _input_points[i] = PointInfo(x,y,false);
        if (max_abs_y < qAbs(y)) {
            x_of_max_abs = x;
            max_abs_y = qAbs(y);
        }
    }

    PrepareToLearning(x_of_max_abs, max_abs_y);
}

double CntlBuilder::CalcSumError()
{
    /* Суммарная ошибка считается на основе входных точек (не изменённых в процессе обучения!)
     и выхода контроллера */
    double sum_error(0);
    for (int i = 0; i < _input_points.size(); ++i) {
        double x = _input_points[i].x, y = _input_points[i].y;
        double y_cntl = _cntl(x);
        sum_error += (y - y_cntl)*(y - y_cntl);
    }
    return sum_error;
}

void CntlBuilder::GetInputPointsX(QVector<double> &x_vals) const
{
    x_vals.resize(_input_points.size());
    for (int i = 0; i < _input_points.size(); ++i) {
        x_vals[i] = _input_points[i].x;
    }
}

void CntlBuilder::GetInputPointsY(QVector<double> &y_vals) const
{
    y_vals.resize(_input_points.size());
    for (int i = 0; i < _input_points.size(); ++i) {
        y_vals[i] = _input_points[i].y;
    }
}

void CntlBuilder::GetRestInputPointsX(QVector<double> &x_vals) const
{
    x_vals.clear();
    for (int i = 0; i < _input_points.size(); ++i) {
        if (_input_points[i].is_removed == false) {
            x_vals.push_back(_input_points[i].x);
        }
    }
}

void CntlBuilder::GetRestInputPointsY(QVector<double> &y_vals) const
{
    y_vals.clear();
    for (int i = 0; i < _input_points.size(); ++i) {
        if (_input_points[i].is_removed == false) {
            y_vals.push_back(_input_points[i].y);
        }
    }
}

void CntlBuilder::GetRecogLinePoints(QVector<double> &x_vals, QVector<double> &y_vals) const
{
    x_vals.resize(_recog_line_points_ptrs.size());
    y_vals.resize(_recog_line_points_ptrs.size());
    for (int i = 0; i < _recog_line_points_ptrs.size(); ++i) {
        x_vals[i] = _recog_line_points_ptrs[i]->x;
        y_vals[i] = _recog_line_points_ptrs[i]->y;
    }
}

bool CntlBuilder::BuildNextMemFunc()
{
    if (_is_ready_to_build == false) return false;

    if (_not_removed_points_cnt < MIN_POINTS_FOR_LINE_DEF) {
        qDebug() << "It remains too few points!";
        return false; //Условие остановки обучения
    }

    RecogNextLine();

    PickPointsFromRecogLine();

    if (_have_to_use_filter == true) {
        FilterRecogLinePoints();
    }

    if (_recog_line_points_ptrs.size() <= MIN_POINTS_FOR_LINE_DEF) {
        if (_repeated_calls < MAX_REPEATED_CALLS) {
            ++_repeated_calls;
            qDebug() << "repeated calls: " << _repeated_calls;
            MarkPointsFromRecogLineAsRemoved();
            return BuildNextMemFunc(); //Рекурсивный вызов
        } else {
            if (_have_to_use_filter == true) {
                _have_to_use_filter = false;
                _repeated_calls = 0;
                qDebug() << "filter was disabled!";
                return BuildNextMemFunc(); //Рекурсивный вызов
            } else {
                return false;   //Остановка обучения
            }
        }
    } else {
        _repeated_calls = 0;
    }

    BuildMemFunc();

    MarkPointsFromRecogLineAsRemoved();

    ++_steps_done;
    qDebug() << "Step #" << _steps_done << "done";

    return true;
}

void CntlBuilder::BuildCntl()
{
    //Построить контроллер на основе текущего содержимого _mem_funcs

    if (_mem_funcs.size() == 0) return;

    const int n_rows_in_A = _input_points.size(),
              n_cols_in_A = 2 * _mem_funcs.size();
    arma::mat A(n_rows_in_A, n_cols_in_A);
    arma::vec B(n_rows_in_A);

    //Заполнение A
    for (int row = 0; row < A.n_rows; ++row) {
        int point_id = row;
        double x = _input_points[point_id].x;
        double mem_funcs_sum = 0;
        for (int i = 0; i < _mem_funcs.size(); ++i) {
            mem_funcs_sum += _mem_funcs[i](x);
        }
        for (int col = 0; col < A.n_cols; ++col) {
            int mem_func_id = col / 2;
            double value = _mem_funcs[mem_func_id](x) / mem_funcs_sum;
            value = (col % 2 == 0)? value * x: value;
            A(row,col) = value;
        }
    }

    //Заполнение B
    for (int row = 0; row < B.n_rows; ++row) {
        int point_id = row;
        B(row) = _input_points[point_id].y;
    }

    //Решение системы уравнений
    arma::vec X = solve(A,B);

    //Формирование следствий правил
    QVector<UnaryFunc> conseq_funcs(_mem_funcs.size());
    for (int i = 0; i < conseq_funcs.size(); ++i) {
        int a_id = 2*i, b_id = 2*i+1;
        double a = X(a_id), b = X(b_id);
        UnaryFunc conseq_func = std::function<double(double)>([a,b](double x)->double { return a*x + b; });
        conseq_funcs[i] = conseq_func;
    }

    //Добавление правил
    for (int i = 0; i < _mem_funcs.size(); ++i) {
        _cntl.AddRule(_mem_funcs[i], conseq_funcs[i]);
    }
}

void CntlBuilder::BuildAll()
{
    bool is_done = true;
    while (is_done == true) {
        is_done = BuildNextMemFunc();
    }
    BuildCntl();
}

QVector<CntlBuilder::DistCluster> CntlBuilder::KMeansByDist()
{
    assert(MIN_POINTS_FOR_LINE_DEF <= _recog_line_points_ptrs.size());

    //исходный контейнер точек отсортирован => отсортирован контейнер с указателями
    QVector<double> dist_vals(_recog_line_points_ptrs.size() - 1);
    for (int i = 0; i < dist_vals.size(); ++i) {
        double x1 = _recog_line_points_ptrs[i]->x, y1 = _recog_line_points_ptrs[i]->y,
               x2 = _recog_line_points_ptrs[i + 1]->x, y2 = _recog_line_points_ptrs[i + 1]->y;
        double dist = qSqrt( (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1) );
        dist_vals[i] = dist;
    }

    //Проверка на случай, когда все расстояния между соседними точками малы
    QVector<DistCluster> prev_dist_labels(dist_vals.size(), dcSHORT);
    auto minmax_it = std::minmax_element(dist_vals.begin(), dist_vals.end());
    double min_dist = *(minmax_it.first), first_max_dist = *(minmax_it.second);
    double minmax_dist = qAbs( first_max_dist - min_dist );
    const double dist_eps = 0.1;    //! магическое число
    if (minmax_dist < dist_eps) {
        //есть только один кластер
        return prev_dist_labels;
    }

    //Определение начальных центроидов
    double second_max_dist = min_dist;
    for (int i = 0; i < dist_vals.size(); ++i) {
        if (dist_vals[i] < first_max_dist  && second_max_dist < dist_vals[i]) {
            second_max_dist = dist_vals[i];
        }
    }
    double short_mid = min_dist, long_mid = qAbs( second_max_dist - min_dist) < dist_eps
            ? first_max_dist
            : second_max_dist;

    //Итерации алгоритма
    QVector<DistCluster> curr_dist_labels(dist_vals.size(), dcSHORT);
    int short_size = 0, long_size = 0;
    while (true) {
        short_size = 0; long_size = 0;
        double short_dist_sum = 0, long_dist_sum = 0;
        for (int i = 0; i < dist_vals.size(); ++i) {
            double dist_to_short = qAbs( dist_vals[i] - short_mid ),
                   dist_to_long = qAbs( dist_vals[i] - long_mid);
            if (dist_to_short <= dist_to_long) {
                curr_dist_labels[i] = dcSHORT;
                ++short_size;
                short_dist_sum += dist_vals[i];
            } else {
                curr_dist_labels[i] = dcLONG;
                ++long_size;
                long_dist_sum += dist_vals[i];
            }
        }
        //Проверка условий остановки
        if (short_size == 0) return curr_dist_labels;
        if (long_size == 0) return curr_dist_labels;

        bool is_anything_changed = false;
        for (int i = 0; i < curr_dist_labels.size(); ++i) {
            int curr_label = int(curr_dist_labels[i]) , prev_label = int(prev_dist_labels[i]);
            if (curr_label != prev_label) {
                is_anything_changed = true;
                break;
            }
        }
        //Проверка условия остановки
        if (is_anything_changed == false) return curr_dist_labels;

        for (int i = 0; i < prev_dist_labels.size(); ++i) {
            prev_dist_labels[i] = curr_dist_labels[i];
        }
        short_mid = 1.0/short_size * short_dist_sum;
        long_mid = 1.0/long_size * long_dist_sum;
    }
    return curr_dist_labels;
}

void CntlBuilder::PrepareToLearning(double x_of_max_abs_y, double max_abs_y)
{
    assert(0 <= max_abs_y);

    _hough.Init(x_of_max_abs_y, max_abs_y);
    AscSortPointsByX(_input_points);
    _not_removed_points_cnt = _input_points.size();
    _mem_funcs.clear();
    _cntl.Clear();
    _steps_done = 0;

    _repeated_calls = 0;
    _have_to_use_filter = true;

    _is_ready_to_build = true;
}

void CntlBuilder::RecogNextLine()
{
    _hough.Clear();
    for (int i = 0; i < _input_points.size(); ++i) {
        if (_input_points[i].is_removed == false) {
            double x = _input_points[i].x, y = _input_points[i].y;
            _hough.AddPoint(x,y);
        }
    }
    _recog_line_angle_coef = _hough.GetLineAngleCoef();
    _recog_line_shift = _hough.GetLineShift();
}

void CntlBuilder::PickPointsFromRecogLine()
{
    _recog_line_points_ptrs.clear();
    for (PointInfo &point: _input_points) {
        double x = point.x, y = point.y;
        if (point.is_removed == true) continue;
        if (_hough.IsPointFromRecogLine(x,y) == true) {
            _recog_line_points_ptrs.push_back(&point);
        }
    }
}

void CntlBuilder::AscSortPointsByX(QVector<PointInfo> &points)
{
    std::sort(points.begin(), points.end(), [](const PointInfo &p1, const PointInfo &p2)->bool { return p1.x < p2.x; });
}

void CntlBuilder::FilterRecogLinePoints()
{
    if (_recog_line_points_ptrs.size() < MIN_POINTS_FOR_LINE_DEF) return;

    QVector<DistCluster> dist_labels = KMeansByDist();

    int ret_start_pos = 0, ret_part_size = 1;
    int start_pos = 0, end_pos = 0, part_size = 0;
    for (int i = 0; i < dist_labels.size(); ++i) {
        if (dist_labels[i] == dcLONG) {
            end_pos = i;
            part_size = end_pos - start_pos + 1;
            if (ret_part_size < part_size) {
                ret_start_pos = start_pos;
                ret_part_size = part_size;
            }
            start_pos = end_pos + 1;
        }
    }
    end_pos = _recog_line_points_ptrs.size() - 1;
    part_size = end_pos - start_pos + 1;
    if (ret_part_size < part_size) {
        ret_start_pos = start_pos;
        ret_part_size = part_size;
    }

    _recog_line_points_ptrs = _recog_line_points_ptrs.mid(ret_start_pos, ret_part_size);
}

void CntlBuilder::BuildMemFunc()
{
    QVector<double> x_vals(_recog_line_points_ptrs.size());
    for (int i = 0; i < x_vals.size(); ++i) {
        x_vals[i] = _recog_line_points_ptrs[i]->x;
    }
    //Функция нормального распределения, контроллер получается всюду определённым
    UnaryFunc m_func = SugenoCntl::GenNormalFunc(x_vals);
    _mem_funcs.push_back(m_func);
}

void CntlBuilder::MarkPointsFromRecogLineAsRemoved()
{
    int removed_points_cnt = 0;
    for (PointInfo &point: _input_points) {
        for (const PointInfo *point_ptr: _recog_line_points_ptrs) {
            if (&point == point_ptr) {
                point.is_removed = true;
                ++removed_points_cnt;
            }
        }
    }
    assert(removed_points_cnt == _recog_line_points_ptrs.size());
    _not_removed_points_cnt -= removed_points_cnt;
}
