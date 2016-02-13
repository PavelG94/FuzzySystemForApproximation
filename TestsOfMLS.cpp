#include <QtMath>
#include <gtest/gtest.h>
#include "MLS.h"

TEST(MLS, BookTestOne)
{
    //источник: http://www.cleverstudents.ru/articles/mnk.html
    QVector<double> x_vals = {0, 1, 2, 4, 5};
    QVector<double> y_vals = {2.1, 2.4, 2.6, 2.8, 3};
    double a = 0, b = 0;
    ApplyMLS(a, b, x_vals, y_vals);
    const double eps = 0.1;
    EXPECT_LT(qAbs(a - 0.165), eps) << "Bad value of a coef";
    EXPECT_LT(qAbs(b - 2.184), eps) << "Bad value of b coef";
}

TEST(MLS, BookTestTwo)
{
    //источник: http://multitest.semico.ru/mnk.htm
    QVector<double> x_vals = {1, 2, 3, 4, 5, 6};
    QVector<double> y_vals = {5.2, 6.3, 7.1, 8.5, 9.2, 10};
    double a = 0, b = 0;
    ApplyMLS(a, b, x_vals, y_vals);
    const double eps = 0.1;
    EXPECT_LT(qAbs(a - 0.98), eps) << "Bad value of a coef";
    EXPECT_LT(qAbs(b - 4.3), eps) << "Bad value of b coef";
}
