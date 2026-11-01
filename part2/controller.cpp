#include "controller.h"

Controller::Controller(double _Kp)
{
    Kp     = _Kp;
    out_lo = 0.0;
    out_hi = 100.0;
}

void Controller::set_limits(double lo, double hi)
{
    out_lo = lo;
    out_hi = hi;
}

double Controller::clamp(double u)
{
    if (u > out_hi) { u = out_hi; }
    if (u < out_lo) { u = out_lo; }
    return u;
}
