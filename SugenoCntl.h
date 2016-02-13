#ifndef SUGENOCNTL_H
#define SUGENOCNTL_H

#include <QVector>
#include "UnaryFunc.h"

struct Rule
{
    UnaryFunc m_func;
    UnaryFunc linear_func;
};

class SugenoCntl : public UnaryFuncBase
{
public:
    static UnaryFunc GenTriangularFunc(const QVector<double> &values);
    static UnaryFunc GenNormalFunc(const QVector<double> &values);

    SugenoCntl();

    void AddRule(UnaryFunc m_func, UnaryFunc linear_func);
    double operator()(double x) override;
    bool IsLastResValid() const override;

    void Clear();

private:
    QVector<Rule> rules;
    bool _validity_flag;
};

#endif // SUGENOCNTL_H
