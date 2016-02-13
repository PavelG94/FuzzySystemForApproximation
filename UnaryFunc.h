#ifndef UNARYFUNC_H
#define UNARYFUNC_H

#include <functional>
#include "UnaryFuncBase.h"

class UnaryFunc : public UnaryFuncBase
{
public:
    UnaryFunc() = default;
    UnaryFunc(const std::function<double(double)>& f);

    void SetFunc(const std::function<double(double)>& f);
    double operator()(double x) override;
    bool IsLastResValid() const override;

private:
    std::function<double(double)> _f;
    bool _validity_flag;
};

#endif // UNARYFUNC_H
