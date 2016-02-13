#include <QtMath>
#include <QVector>
#include <gtest/gtest.h>
#include "HoughTransform.h"

class HoughTransformTest : public ::testing::Test
{
public:
    HoughTransform hough;
};

TEST_F(HoughTransformTest, Init)
{
    double x = 2, y = -5;
    double arg_of_max_abs = x, value_of_max_abs = qAbs(y), radius_step = 0.1;
    hough.Init(arg_of_max_abs, value_of_max_abs, radius_step);
    int right_columns_cnt = 61;
    ASSERT_EQ(hough.ColumnsCnt(), right_columns_cnt) << "Wrong columns count";
}

TEST_F(HoughTransformTest, Work)
{
    //Ф-ия y = -x с небольшими погрешностями
    QVector<double> x_vals = {-2, -1, 0, 1, 1.5};
    QVector<double> y_vals = {2, 1, 0.2, -1, -1.2};

    double arg_of_max_abs = 5, value_of_max_abs = -5, radius_step = 0.1;
    hough.Init(arg_of_max_abs, value_of_max_abs, radius_step);
    for (int i = 0; i < x_vals.size(); ++i) {
        hough.AddPoint(x_vals[i], y_vals[i]);
    }
    double a = hough.GetLineAngleCoef(), b = hough.GetLineShift();
    const double eps = 0.5;
    EXPECT_LT(qAbs(a - (-1)), eps);
    EXPECT_LT(qAbs(b - 0), eps);
}
