#include <cassert>
#include <QDebug>
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _input_points_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::blue, _INPUT_POINTS_LEGEND, QCPScatterStyle::ssCircle);
    _cntl_output_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::yellow, _CNTL_OUTPUT_LEGEND, QCPScatterStyle::ssCircle);
    _recog_line_points_draw_info = DrawInfo(DrawInfo::tSCATTER, Qt::red, _RECOG_LINE_POINTS_LEGEND, QCPScatterStyle::ssCross);
    _line_points_draw_info = DrawInfo(DrawInfo::tLINE, Qt::black, _LINE_POINTS_LEGEND);

    InitMainWindow();
}

MainWindow::~MainWindow()
{
    delete _tool_bar;
    delete _plot_widget;
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

QMap<double,double> MainWindow::CalcLinePointsForDraw(double angle_coef, double line_shift)
{
    QMap<double,double> line_points;
    QMap<double,double> input_points = _builder->GetInputPoints();
    QMapIterator<double,double> it(input_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key();
        double y = angle_coef * x + line_shift;
        line_points.insert(x,y);
    }
    return line_points;
}

QMap<double, double> MainWindow::CalcCntlPointsForDraw()
{
    SugenoCntl &cntl = _builder->GetController();
    QMap<double,double> input_points = _builder->GetInputPoints();
    QMap<double,double> cntl_points;
    QMapIterator<double,double> it(input_points);
    while (it.hasNext()) {
        it.next();
        double x = it.key();
        double y_cntl = cntl(x);
        cntl_points.insert(x,y_cntl);
    }
    return cntl_points;
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
    QMap<double, double> input_points = _builder->GetInputPoints();
    AddGraphOnPlot(input_points, _input_points_draw_info);
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
}

void MainWindow::AddGraphOnPlot(const QMap<double,double> &points, const DrawInfo &draw_info)
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

    QMap<double,double> input_points = _builder->GetInputPoints();

    QMap<double,double> cntl_points = CalcCntlPointsForDraw();

    double angle_coef = _builder->GetRecogLineAngleCoef(), line_shift = _builder->GetRecogLineShift();
    QMap<double,double> line_points = CalcLinePointsForDraw(angle_coef, line_shift);

    QMap<double,double> recog_line_points = _builder->GetRecogLinePoints();

    ClearPlot();
    AddGraphOnPlot(input_points, _input_points_draw_info);
    AddGraphOnPlot(recog_line_points, _recog_line_points_draw_info);
    AddGraphOnPlot(cntl_points, _cntl_output_draw_info);
    AddGraphOnPlot(line_points, _line_points_draw_info);
    RedrawPlot();

    qDebug() << "Вид распознанной прямой: " << QString("%1*x + %2").arg(angle_coef).arg(line_shift);
}

void MainWindow::DrawResultInfo()
{
    QMap<double,double> input_points = _builder->GetInputPoints();
    QMap<double,double> cntl_points = CalcCntlPointsForDraw();

    ClearPlot();
    AddGraphOnPlot(input_points, _input_points_draw_info);
    AddGraphOnPlot(cntl_points, _cntl_output_draw_info);
    RedrawPlot();

    double sum_error = _builder->CalcSumError();
    qDebug() << "Суммарная ошибка работы контроллера: " << sum_error;
}
