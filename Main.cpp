#include <QApplication>
#include <QtMath>
#include <gtest/gtest.h>

#include "MainWindow.h"

int RunTests(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

int RunProg(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow win;
    UnaryFunc f([](double x)->double { return qSin(x)/x; });
    QCPRange data_x_range(-10,10);
    const double step = 0.1;
    win.SetData(f, data_x_range, step);
    QCPRange draw_x_range(-10,10), draw_y_range(-3,3);
    win.SetAxisRanges(draw_x_range, draw_y_range);
    win.show();

    return app.exec();
}

int main(int argc, char *argv[])
{
    return RunProg(argc, argv);
}
