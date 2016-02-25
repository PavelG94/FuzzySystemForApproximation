#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QToolBar>
#include <QMainWindow>

#include "qcustomplot.h"
#include "UnaryFunc.h"
#include "SugenoCntl.h"
#include "CntlBuilder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct DrawInfo
    {
        DrawInfo(QCPGraph::LineStyle line_style, QCPScatterStyle::ScatterShape scatter_shape, QColor color, const QString& legend)
            : line_style(line_style), scatter_shape(scatter_shape), color(color), legend(legend) { }
        QCPGraph::LineStyle line_style;
        QCPScatterStyle::ScatterShape scatter_shape;
        QColor color;
        QString legend;
    };
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetData(UnaryFunc &f, const QCPRange &x_range, double step);
    void SetData(const QVector<double> &x_vals, const QVector<double> &y_vals);
    void SetAxisRanges(const QCPRange &x_range, const QCPRange &y_range);

public slots:
    void StepButtonSlot();
    void ResultButtonSlot();

private:
    void InitToolBar();
    void InitPlotWidget(const QCPRange &x_range, const QCPRange &y_range);
    void InitMainWindow();
    void PrepareToLearning();
    void RemovePointsFrom(QMap<double,double> &points_for_remove, QMap<double,double> &from_points);
    QMap<double,double> CalcValuesOnTheSameArgs(UnaryFuncBase &f, const QMap<double,double> &points);

    void AddGraphOnPlot(const QMap<double,double> &points, const DrawInfo info);
    void DrawStepInfo(const QMap<double,double> &small_error_points, const QMap<double,double>& recog_line_points, double recog_line_angle_coef, double recog_line_shift);
    void DrawResultInfo();
    void RedrawPlot();
    void ClearPlot();

private:
    const int _MAX_LEARNING_STEPS = 100;
    int _learning_steps_done;

    QMap<double, double> _in_points;
    QMap<double, double> _modified_in_points;
    SugenoCntl _cntl;
    CntlBuilder _builder;
    bool _finish_status;

    const QString _INPUT_LEGEND = "Входные точки";
    const QString _OUTPUT_LEGEND = "Выход контроллера";
    QToolBar *_tool_bar = nullptr;
    QCustomPlot *_plot_widget = nullptr;
};

#endif // MAINWINDOW_H
