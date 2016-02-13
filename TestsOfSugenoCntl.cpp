#include <QVector>
#include <gtest/gtest.h>
#include "SugenoCntl.h"

TEST(SugenoCntlTest, TriangularGen)
{
    QVector<double> x_vals = {1, 2, 3, 4, 5, 6};
    UnaryFunc f = SugenoCntl::GenTriangularFunc(x_vals);
    EXPECT_EQ(f(-2), 0);
    EXPECT_EQ(f(-1.5), 0);

    EXPECT_LT(0, f(0));
    EXPECT_EQ(f(3.5), 1);
    EXPECT_LT(0, f(8));

    EXPECT_EQ(f(8.5), 0);
    EXPECT_EQ(f(9), 0);
}

TEST(SugenoCntlTest, NormalGen)
{
    QVector<double> x_vals = {1, 2, 3, 4, 5, 6};
    UnaryFunc f = SugenoCntl::GenTriangularFunc(x_vals);
    const double a = 3.5, b = 6.2665706865775, eps = 0.1;
    EXPECT_LT(f(a - b), eps);
    EXPECT_EQ(f(a), 1);
    EXPECT_LT(f(a + b), eps);
}
