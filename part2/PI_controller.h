#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

#include "controller.h"

// u = Kp * (e + E / Ti),   E = sum of e * T
// Anti-windup: E is not updated while the output is saturated.
class PI_controller : public Controller
{
public:
    PI_controller(double _Kp, double _Ti, double _T);

    void init() override;                                   // clears the integral
    double update(double ref, double actual) override;

    double Ti;      // integration time [s], 0 = no integral (P only)
    double T;       // time step [s], one control window

private:
    double integral;    // E, sum of e * T [rpm*s]
};

#endif
