#include <cmath>
#include <cassert>
#include <algorithm>

#include "SugenoCntl.h"

UnaryFunc SugenoCntl::GenTriangularFunc(const QVector<double> &values)
{
    auto it_to_min = std::min_element(values.begin(), values.end());
    auto it_to_max = std::max_element(values.begin(), values.end());
    assert(it_to_min != values.end() || it_to_max != values.end());
    double a = (*it_to_max + *it_to_min)/ 2,
            b = *it_to_max - *it_to_min;
    assert(b != 0);
    auto f = [a,b](double x)->double { return std::max(1 - std::abs((x - a)/b), 0.0); };
    return std::function<double(double)>(f);
}

UnaryFunc SugenoCntl::GenNormalFunc(const QVector<double> &values)
{
    auto it_to_min = std::min_element(values.begin(), values.end());
    auto it_to_max = std::max_element(values.begin(), values.end());
    assert(it_to_min != values.end() || it_to_max != values.end());
    double a = (*it_to_max + *it_to_min)/ 2,
            b = std::sqrt(M_PI/2) * (*it_to_max - *it_to_min);
    assert(b != 0);
    auto f = [a,b](double x)->double { return std::pow(M_E, -M_PI * ((x - a)*(x - a))/(b*b)); };
    return std::function<double(double)>(f);
}

SugenoCntl::SugenoCntl()
    : _validity_flag(true) { }

void SugenoCntl::AddRule(UnaryFunc m_func, UnaryFunc linear_func)
{
    Rule rule;
    rule.m_func = m_func;
    rule.linear_func = linear_func;
    _rules.push_back(rule);
}

bool SugenoCntl::IsLastResValid() const
{
    return _validity_flag;
}

void SugenoCntl::Clear()
{
    _rules.clear();
    _validity_flag = true;
}

double SugenoCntl::operator()(double x)
{
    bool has_active_rules = false;
    double numerator(0), denominator(0);
    for (int i = 0; i < _rules.size(); ++i) {
        double m_func_val = _rules[i].m_func(x);
        bool is_active = m_func_val > 0;
        if (is_active) {
            has_active_rules = true;
            numerator += m_func_val * _rules[i].linear_func(x);
            denominator += m_func_val;
        }
    }
    if (has_active_rules == true) {
        _validity_flag = true;
        return numerator / denominator;
    } else {
        _validity_flag = false;
        return 0;
    }
}
