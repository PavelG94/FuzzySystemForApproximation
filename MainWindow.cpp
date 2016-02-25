#include <cassert>
#include <QDebug>
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    InitMainWindow();
}
MainWindow::~MainWindow()
{
    delete _tool_bar;
    delete _plot_widget;
}

void MainWindow::SetData(UnaryFunc &f, const QCPRange &x_range, double step)
{
    assert(0 < step);
    int points_cnt = (x_range.upper - x_range.lower) / step; // [lower, upper)
    assert(1 < points_cnt);
    QCPRange y_range;
    y_range.lower = f(x_range.lower); y_range.upper = f(x_range.lower);
    for (int i = 0; i < points_cnt; ++i) {
        double x = x_range.lower + i*step;
        double y  = f(x);
        if (f.IsLastResValid() == true) {
            //вставка без повторов
            _in_points.insert(x, y);
            _modified_in_points.insert(x, y);
        }
        if (y < y_range.lower) y_range.lower = y;
        else if (y > y_range.upper) y_range.upper = y;
    }
    InitPlotWidget(x_range, y_range);
    PrepareToLearning();
    _learning_steps_done = 0;
    _finish_status = false;
}

void MainWindow::SetData(const QVector<double> &x_vals, const QVector<double> &y_vals)
{
    //Возможен случай, когда во входной последовательности присутствует шум
    assert(1 < x_vals.size());
    assert(y_vals.size() == x_vals.size());

    QCPRange x_range, y_range;
    x_range.lower = x_vals[0]; x_range.upper = x_vals[0];
    y_range.lower = y_vals[0]; y_range.upper = y_vals[0];
    for (int i = 0; i < x_vals.size(); ++i) {
        double x = x_vals[i], y = y_vals[i];
        //вставка без повторов
        _in_points.insert(x,y);
        _modified_in_points.insert(x,y);

        if (x < x_range.lower) x_range.lower = x;
        else if (x > x_range.upper) x_range.upper = x;

        if (y < y_range.lower) y_range.lower = y;
        else if (y > y_range.upper) y_range.upper = y;
    }
    InitPlotWidget(x_range, y_range);
    PrepareToLearning();
    _learning_steps_done = 0;
    _finish_status = false;
}

void MainWindow::SetAxisRanges(const QCPRange &x_range, const QCPRange &y_range)
{
    InitPlotWidget(x_range, y_range);
}

void MainWindow::StepButtonSlot()
{
    if (_finish_status == true) {
        DrawResultInfo();
        return;
    }
    bool is_step_done = _builder.BuildStep();
    if(is_step_done) {
        ++_learning_steps_done;
        QMap<double,double> small_dist_to_recog_line_points = _builder.GetSmallDistToRecogLinePoints();
        //Входное множество точек не меняется на протяжении всего обучения!
        double a = _builder.GetAngleCoefOfRecogLine(),
               b = _builder.GetShiftOfRecogLine();
        UnaryFunc line = std::function<double(double)>([a,b](double x)->double { return a*x + b; });
        QMap<double,double> line_points = CalcValuesOnTheSameArgs(line, _in_points);
        DrawStepInfo(small_dist_to_recog_line_points, line_points, a, b);
        _modified_in_points = _builder.GetModifiedInPoints();
        _finish_status = (_MAX_LEARNING_STEPS <= _learning_steps_done);
    } else {
       DrawResultInfo();
       _finish_status = true;
    }
}

void MainWindow::ResultButtonSlot()
{
    while (_finish_status == false) {
        ++_learning_steps_done;
        bool is_step_done = _builder.BuildStep();
        _finish_status = (is_step_done == false) || (_MAX_LEARNING_STEPS <= _learning_steps_done);
    }
    DrawResultInfo();
    _finish_status = true;
}

void MainWindow::InitToolBar()
{
    _tool_bar = new QToolBar;
    _tool_bar->setMovable(false);
    _tool_bar->setFloatable(false);

    _tool_bar->addAction("Step", this, SLOT(StepButtonSlot()));
    _tool_bar->addAction("Result", this, SLOT(ResultButtonSlot()));
}

void MainWindow::InitPlotWidget(const QCPRange &x_range, const QCPRange &y_range)
{
    ClearPlot();

    //Легенда
    _plot_widget->legend->setVisible(true);
    _plot_widget->legend->setBrush(QBrush(QColor(255,255,255,230)));

    //Изменение расположения легенды
    _plot_widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

    _plot_widget->xAxis->setLabel("x");
    _plot_widget->yAxis->setLabel("y");

    //Диапазоны значений на осях
    _plot_widget->xAxis->setRange(x_range);
    _plot_widget->yAxis->setRange(y_range);

    if (_in_points.isEmpty() == false) {
        AddGraphOnPlot(_in_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCircle, Qt::blue, _INPUT_LEGEND));
        RedrawPlot();
    }
}

void MainWindow::InitMainWindow()
{
    const int width = 850, height = 600;
    this->setFixedSize(width, height);
    this->setWindowTitle(" ");

    InitToolBar();
    _plot_widget = new QCustomPlot;
    this->addToolBar(Qt::TopToolBarArea, _tool_bar);
    this->setCentralWidget(_plot_widget);
}

void MainWindow::PrepareToLearning()
{
    double max_abs_arg = 0, max_abs_value = 0;
    QMapIterator<double,double> it(_in_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key(), y = it.value();
        if (max_abs_value < qAbs(y)) {
            max_abs_arg = qAbs(x);
            max_abs_value = qAbs(y);
        }
    }
    _builder.SetData(&_cntl, max_abs_arg, max_abs_value);
    _builder.SetInput(_in_points);
}

void MainWindow::RemovePointsFrom(QMap<double, double> &points_for_remove, QMap<double, double> &from_points)
{
    QMapIterator<double,double> it(points_for_remove);
    while (it.hasNext()) {
        it.next();
        double x = it.key();
        from_points.remove(x);
    }
}

QMap<double, double> MainWindow::CalcValuesOnTheSameArgs(UnaryFuncBase &f, const QMap<double, double> &points)
{
    QMap<double,double> out_points;
    QMapIterator<double,double> it(points);
    while (it.hasNext()) {
        it.next();
        double x = it.key();
        double y = f(x);
        out_points.insert(x,y);
    }
    return out_points;
}

void MainWindow::AddGraphOnPlot(const QMap<double,double> &points, const DrawInfo info)
{
    QCPGraph *graph = _plot_widget->addGraph();
    QVector<double> x_vals(points.size()), y_vals(points.size());
    QMapIterator<double,double> it(points);
    for(int i = 0; i < points.size(); ++i) {
        it.next();
        x_vals[i] = it.key();
        y_vals[i] = it.value();
    }
    graph->setData(x_vals, y_vals);
    graph->setLineStyle(info.line_style);
    if (info.line_style == QCPGraph::lsNone) {
        double scatter_size = (info.scatter_shape == QCPScatterStyle::ssCircle)? 4: 10;
        //QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size);
        graph->setScatterStyle(QCPScatterStyle(info.scatter_shape, info.color, info.color, scatter_size));
    } else {
        const int pen_width = 1;
        QPen pen(info.color);
        pen.setWidth(pen_width);
        graph->setPen(pen);
    }
    graph->setName(info.legend);
}

void MainWindow::DrawStepInfo(const QMap<double, double> &small_dist_to_recog_line_points, const QMap<double, double> &recog_line_points, double recog_line_angle_coef, double recog_line_shift)
{
    QMap<double,double> cntl_points = CalcValuesOnTheSameArgs(_cntl, _in_points);
    QCPRange new_y_range = _plot_widget->yAxis->range();
    QMapIterator<double,double> it(cntl_points);
    while (it.hasNext()) {
        it.next();
        double y = it.value();
        if (y < new_y_range.lower) new_y_range.lower = y - 1;
        else if (new_y_range.upper < y) new_y_range.upper = y + 1;
    }
    _plot_widget->yAxis->setRange(new_y_range);

    ClearPlot();
    AddGraphOnPlot(_modified_in_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCircle, Qt::blue, _INPUT_LEGEND));
    AddGraphOnPlot(small_dist_to_recog_line_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCross, Qt::red, "Точки, которые определили распознанную прямую"));
    AddGraphOnPlot(cntl_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCircle, Qt::yellow, _OUTPUT_LEGEND));
    AddGraphOnPlot(recog_line_points, DrawInfo(QCPGraph::lsLine, QCPScatterStyle::ssNone, Qt::black,
                                         QString("Распознанная прямая %1x + %2").arg(recog_line_angle_coef).arg(recog_line_shift)));
    RedrawPlot();
}

void MainWindow::DrawResultInfo()
{
    QMap<double,double> cntl_points = CalcValuesOnTheSameArgs(_cntl, _in_points);
    QCPRange new_y_range = _plot_widget->yAxis->range();
    QMapIterator<double,double> it(cntl_points);
    while (it.hasNext()) {
        it.next();
        double y = it.value();
        if (y < new_y_range.lower) new_y_range.lower = y - 1;
        else if (new_y_range.upper < y) new_y_range.upper = y + 1;
    }
    _plot_widget->yAxis->setRange(new_y_range);

    ClearPlot();
    AddGraphOnPlot(_in_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCircle, Qt::blue, _INPUT_LEGEND));
    AddGraphOnPlot(cntl_points, DrawInfo(QCPGraph::lsNone, QCPScatterStyle::ssCircle, Qt::yellow, _OUTPUT_LEGEND));
    RedrawPlot();

    double sum_error = CntlBuilder::CalcSumError(_cntl, _in_points);
    qDebug() << "Sum error is " << sum_error;
}

void MainWindow::RedrawPlot()
{
    _plot_widget->replot();
}

void MainWindow::ClearPlot()
{
    _plot_widget->clearGraphs();
}
