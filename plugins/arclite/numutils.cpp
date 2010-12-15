#include "utils.hpp"

int round(double d) {
  double a = fabs(d);
  int res = static_cast<int>(a);
  double frac = a - res;
  if (frac >= 0.5)
    res++;
  if (d >= 0)
    return res;
  else
    return -res;
}
