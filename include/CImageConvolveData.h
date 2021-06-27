#ifndef CImageConvolveData_H
#define CImageConvolveData_H

struct CImageConvolveData {
  // TODO: edge mode

  using Kernel = std::vector<double>;

  CImageConvolveData() { }

  Kernel kernel;
  int    xsize         = -1;
  int    ysize         = -1;
  double divisor       = -1;
  double bias          = -1;
  bool   preserveAlpha = false;
};

#endif
