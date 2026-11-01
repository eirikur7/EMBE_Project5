#include "PI_controller.h"

PI_controller::PI_controller(double _Kp, double _Ti, double _T) : Controller(_Kp)
{
    Ti = _Ti;
    T  = _T;
    integral = 0.0;
}

void PI_controller::init()
{
    integral = 0.0;
}

double PI_controller::update(double ref, double actual)
{
    double e = ref - actual;

    // Ti = 0 means no integral, behave like a P controller
    if (Ti <= 0.0)
        return clamp(Kp * e);

    // Try the new integral
    double new_integral = integral + e * T;
    double u = Kp * (e + new_integral / Ti);

    // Saturated high: keep the old integral (anti-windup)
    if (u > out_hi) { return out_hi; }

    // Saturated low: let the integral run down, but not below the lower limit,
    // so it does not keep the motor on after the reference goes to 0
    if (u < out_lo)
    {
        integral = new_integral;
        double integral_min = out_lo * Ti / Kp;
        if (integral < integral_min) { integral = integral_min; }
        return out_lo;
    }

    // Not saturated: keep the new integral
    integral = new_integral;
    return u;
}
