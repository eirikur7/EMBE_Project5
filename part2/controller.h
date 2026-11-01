#ifndef CONTROLLER_H
#define CONTROLLER_H

// Base class for all controllers.
// The motor code only talks to a Controller, so P and PI can be swapped.

class Controller
{
public:
    Controller(double _Kp);
    virtual ~Controller() {}

    virtual void init() {}                                  // reset internal values
    virtual double update(double ref, double actual) = 0;   // every controller has its own

    void set_limits(double lo, double hi);

    double Kp;

protected:
    double clamp(double u);     // keep u inside the limits

    double out_lo;
    double out_hi;
};

#endif
