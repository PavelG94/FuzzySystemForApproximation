#include <cassert>
#include <QDebug>
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _input_points_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::blue, _INPUT_POINTS_LEGEND, QCPScatterStyle::ssCircle);
    //_modif_input_points_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::blue, _MODIF_INPUT_POINTS_LEGEND, QCPScatterStyle::ssCircle);
    _cntl_output_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::yellow, _CNTL_OUTPUT_LEGEND, QCPScatterStyle::ssCircle);
    _recog_line_points_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::red, _RECOG_LINE_POINTS_LEGEND, QCPScatterStyle::ssCross);
    _line_points_draw_info = DrawInfo(DrawInfo::tLINE, Qt::black, _LINE_POINTS_LEGEND);

    InitMainWindow();
}

MainWindow::~MainWindow()
{
    delete _tool_bar;
    delete _plot_widget;
    delete _status_bar;
}

void MainWindow::StepClickedSlot()
{
    assert(_builder != nullptr);
    bool is_step_done = _builder->BuildStep();
    if (is_step_done) {
        DrawStepInfo();
    } else {
        DrawResultInfo();
    }
}

void MainWindow::ResultClickedSlot()
{
    assert(_builder != nullptr);
    _builder->Build();
    DrawResultInfo();
}

QVector<double> MainWindow::CalcLineValuesForDraw(const QVector<double> &x_vals, double angle_coef, double line_shift)
{
    QVector<double> line_vals(x_vals.size());
    for (int i = 0; i < x_vals.size(); ++i) {
        double y = angle_coef * x_vals[i] + line_shift;
        line_vals[i] = y;
    }
    return line_vals;
}

QVector<double> MainWindow::CalcCntlValuesForDraw(const QVector<double> &x_vals)
{
    SugenoCntl &cntl = _builder->GetController();
    QVector<double> y_cntl_vals(x_vals.size());
    for (int i = 0; i < y_cntl_vals.size(); ++i) {
        double x = x_vals[i];
        double y_cntl = cntl(x);
        y_cntl_vals[i] = y_cntl;
    }
    return y_cntl_vals;
}

void MainWindow::RedrawPlot()
{
    _plot_widget->replot();
}

void MainWindow::ClearPlot()
{
    _plot_widget->clearGraphs();
}

void MainWindow::SetAxisRanges(const QCPRange &x_range, const QCPRange &y_range)
{
    InitPlotWidget(x_range, y_range);
}

void MainWindow::SetBuilder(CntlBuilder *builder)
{
    _builder = builder;
}

void MainWindow::DrawInputPoints()
{
    assert(_builder != nullptr);
    QVector<double> input_x_vals, input_y_vals;
    _builder->GetInputPointsX(input_x_vals);
    _builder->GetInputPointsY(input_y_vals);
    AddGraphOnPlot(input_x_vals, input_y_vals, _input_points_draw_info);
    RedrawPlot();
}

void MainWindow::InitToolBar()
{
    _tool_bar = new QToolBar;
    _tool_bar->setMovable(false);
    _tool_bar->setFloatable(false);

    _step_act = _tool_bar->addAction("Step", this, SLOT(StepClickedSlot()));
    _result_act = _tool_bar->addAction("Result", this, SLOT(ResultClickedSlot()));
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

    _status_bar = new QStatusBar;
    this->setStatusBar(_status_bar);
}

void MainWindow::AddGraphOnPlot(const QVector<double> &x_vals, const QVector<double> &y_vals, const DrawInfo &draw_info)
{
    QCPGraph *graph = _plot_widget->addGraph();
    graph->setData(x_vals, y_vals);
    QCPGraph::LineStyle line_style;
    if (draw_info.type == DrawInfo::tLINE) {
        line_style = QCPGraph::lsLine;
    } else if (draw_info.type == DrawInfo::tSCATTER) {
        line_style = QCPGraph::lsNone;
    }
    graph->setLineStyle(line_style);
    if (line_style == QCPGraph::lsNone) {
        double scatter_size = (draw_info.style == QCPScatterStyle::ssCircle)? 4: 10;
        QCPScatterStyle::ScatterShape shape = QCPScatterStyle::ScatterShape(draw_info.style);

        //QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size);
        graph->setScatterStyle(QCPScatterStyle(shape, draw_info.color, draw_info.color, scatter_size));
    } else if (line_style == QCPGraph::lsLine){
        const int pen_width = 1;
        QPen pen(draw_info.color);
        pen.setWidth(pen_width);
        graph->setPen(pen);
    }
    graph->setName(draw_info.legend);
}

void MainWindow::DrawStepInfo()
{
    QVector<double> rest_x_vals, rest_y_vals;
    _builder->GetRestInputPointsX(rest_x_vals);
    _builder->GetRestInputPointsY(rest_y_vals);

    QVector<double> input_x_vals;
    _builder->GetInputPointsX(input_x_vals);

    QVector<double> cntl_y_vals = CalcCntlValuesForDraw(input_x_vals);
    double angle_coef = _builder->GetRecogLineAngleCoef(), line_shift = _builder->GetRecogLineShift();

    //точки, по которым строился нечёткий терм для последнего добавленного правила
    QVector<double> recog_line_x_vals, recog_line_y_vals;
    _builder->GetRecogLinePoints(recog_line_x_vals, recog_line_y_vals);

    //распознанная прямая с параметрами, уточнёнными с помощью МНК
    QVector<double> line_y_vals = CalcLineValuesForDraw(input_x_vals, angle_coef, line_shift);

    ClearPlot();
    AddGraphOnPlot(rest_x_vals, rest_y_vals, _input_points_draw_info);
    AddGraphOnPlot(input_x_vals, cntl_y_vals, _cntl_output_draw_info);
    AddGraphOnPlot(recog_line_x_vals, recog_line_y_vals, _recog_line_points_draw_info);
    AddGraphOnPlot(input_x_vals, line_y_vals, _line_points_draw_info);
    RedrawPlot();

    QString line_info_msg = QString("распознанная прямая: %1*x + %2").arg(angle_coef).arg(line_shift);
    double sum_error = _builder->CalcSumError();
    QString sum_error_msg = QString("суммарная ошибка: %1").arg(sum_error);
    _status_bar->showMessage(line_info_msg + " | " + sum_error_msg);
}

void MainWindow::DrawResultInfo()
{
    QVector<double> input_x_vals, input_y_vals;
    _builder->GetInputPointsX(input_x_vals);
    _builder->GetInputPointsY(input_y_vals);
    QVector<double> cntl_y_vals = CalcCntlValuesForDraw(input_x_vals);

    ClearPlot();
    AddGraphOnPlot(input_x_vals, input_y_vals, _input_points_draw_info);
    AddGraphOnPlot(input_x_vals, cntl_y_vals, _cntl_output_draw_info);
    RedrawPlot();

    double sum_error = _builder->CalcSumError();
    QString sum_error_msg = QString("суммарная ошибка: %1").arg(sum_error);
    _status_bar->showMessage(sum_error_msg);
}
