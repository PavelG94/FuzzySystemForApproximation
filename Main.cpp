#include <random>

#include <QApplication>
#include <QtMath>

#include "CntlBuilder.h"
#include "MainWindow.h"

void GenInputValues(QVector<double> &x_vals, QVector<double> &y_vals,
                    UnaryFunc f, double x_min, double x_max, double step, bool with_noise)
{
    int points_cnt = (x_max - x_min + step) / step; // [x_min, x_max]
    x_vals.resize(points_cnt);
    y_vals.resize(points_cnt);

    std::default_random_engine gen;
    const double mean = 0, stddev = 0.5;
    std::normal_distribution<double> normal_distr(mean,stddev);
    int left_bound = 0, right_bound = 1;
    std::uniform_int_distribution<int> uniform_distr(left_bound,right_bound);
    for (int i = 0; i < points_cnt; ++i) {
        double x = x_min + i*step;
        double y = f(x);
        if (f.IsLastResValid() == false) {
            y = 0;
        } else if (with_noise == true) {
            double noise_val = normal_distr(gen);
            int possib = uniform_distr(gen);
            y = y + possib*noise_val;
        }
        x_vals[i] = x;
        y_vals[i] = y;
    }
}

int RunProg(int argc, char *argv[])
{
    QApplication app(argc, argv);

    UnaryFunc f([](double x)->double {
        double y = (x != 0)? qSin(x)/x: 1;
        //double y = qPow(x,5);
        return y;
    });
    const double x_min = -10, x_max = 10, step = 0.1;

    QVector<double> x_vals, y_vals;
    bool with_noise = false;
    GenInputValues(x_vals, y_vals, f, x_min, x_max, step, with_noise);
    CntlBuilder builder;
    builder.SetData(x_vals, y_vals);

    MainWindow win;
    QCPRange x_range(x_min, x_max), y_range(-3,3);
    //QCPRange x_range(x_min, x_max), y_range(-10000,10000);
    win.SetAxisRanges(x_range, y_range);
    win.SetBuilder(&builder);
    win.DrawInputPoints();
    win.show();

    return app.exec();
}

int main(int argc, char *argv[])
{
    return RunProg(argc, argv);
}
