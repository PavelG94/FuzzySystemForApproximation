#ifndef UNARYFUNCBASE
#define UNARYFUNCBASE

class UnaryFuncBase {
public:
    virtual double operator()(double x) = 0;
    virtual bool IsLastResValid() const = 0;
};

#endif // UNARYFUNCBASE

