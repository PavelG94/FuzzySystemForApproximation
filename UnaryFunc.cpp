#include <QtMath>
#include "UnaryFunc.h"

UnaryFunc::UnaryFunc(const std::function<double (double)> &f)
    : _f(f), _validity_flag(true) { }

void UnaryFunc::SetFunc(const std::function<double (double)> &f)
{
    _f = f;
    _validity_flag = true;
}

bool UnaryFunc::IsLastResValid() const
{
    return _validity_flag;
}

double UnaryFunc::operator()(double x)
{
    double res = _f(x);
    _validity_flag = qIsFinite(res);
    return res;
}

