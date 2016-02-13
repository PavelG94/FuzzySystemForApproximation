#include "MLS.h"

void ApplyMLS(double &a, double &b, const QVector<double> &x_vals, const QVector<double> &y_vals)
{
    double sum_x(0), sum_y(0), sum_x_mult_y(0), sum_x_mult_x(0);
    for (int i = 0; i < x_vals.size(); ++i) {
        sum_x += x_vals[i];
        sum_y += y_vals[i];
        sum_x_mult_y += x_vals[i]*y_vals[i];
        sum_x_mult_x += x_vals[i]*x_vals[i];
    }
    double n = x_vals.size();
    a = (n * sum_x_mult_y - sum_x * sum_y) / (n * sum_x_mult_x - sum_x*sum_x);
    b = (sum_y - a*sum_x) / n;
}
