#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QString>
#include <QVector>
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
        enum Type {tLINE, tSCATTER};
        DrawInfo() { }
        //style: значение QCPScatterStyle, если type == tLINE; иначе 0;
        DrawInfo(Type type, QColor color, const QString& legend, int style = 0)
            : type(type), color(color), legend(legend), style(style) { }

        Type type;
        QColor color;
        QString legend;
        int style;
    };

    explicit MainWindow(QWidget *parent = 0);

    void SetAxisRanges(const QCPRange &x_range, const QCPRange &y_range);
    void SetBuilder(CntlBuilder *builder);
    void DrawInputPoints();

     ~MainWindow();

private slots:
    void StepClickedSlot();
    void ResultClickedSlot();

private:
    void InitToolBar();
    void InitPlotWidget(const QCPRange &x_range, const QCPRange &y_range);
    void InitMainWindow();

    QVector<double> CalcLineValuesForDraw(const QVector<double> &x_vals, double angle_coef, double line_shift);
    QVector<double> CalcCntlValuesForDraw(const QVector<double> &x_vals);

    void AddGraphOnPlot(const QVector<double> &x_vals, const QVector<double> &y_vals, const DrawInfo &draw_info);
    void DrawStepInfo();
    void DrawResultInfo();
    void RedrawPlot();
    void ClearPlot();

private:
    const QString _INPUT_POINTS_LEGEND = "Вход";
    //const QString _MODIF_INPUT_POINTS_LEGEND = "Изменённый вход";
    const QString _CNTL_OUTPUT_LEGEND = "Выход контроллера";
    const QString _RECOG_LINE_POINTS_LEGEND = "Точки, которые определили распознанную прямую";
    const QString _LINE_POINTS_LEGEND = "Распознанная прямая";
    DrawInfo _input_points_draw_info, _cntl_output_draw_info;
    DrawInfo _recog_line_points_draw_info, _line_points_draw_info;

    CntlBuilder *_builder = nullptr;

    QToolBar *_tool_bar = nullptr;
    QCustomPlot *_plot_widget = nullptr;
    QAction *_step_act = nullptr;
    QAction *_result_act = nullptr;

    QStatusBar *_status_bar;
};

#endif // MAINWINDOW_H
