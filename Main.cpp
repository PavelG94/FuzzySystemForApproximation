#include <QApplication>
#include <QtMath>
#include <gtest/gtest.h>

#include "CntlBuilder.h"
#include "MainWindow.h"

int RunProg(int argc, char *argv[])
{
    QApplication app(argc, argv);

    UnaryFunc f([](double x)->double { return qSin(x)/x; });
    const double x_min = -10, x_max = 10, step = 0.1;

    CntlBuilder builder;
    builder.SetData(f, x_min, x_max, step);

    MainWindow win;
    QCPRange x_range(x_min, x_max), y_range(-3,3);
    win.SetAxisRanges(x_range, y_range);
    win.SetBuilder(&builder);
    win.DrawInputPoints();
    win.show();

    return app.exec();
}

int RunTests(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

int main(int argc, char *argv[])
{
    return RunProg(argc, argv);
}
