#include <CImageFilter.h>
#include <CGaussianBlur.h>
#include <cmath>

void
CImage::
unsharpMask(CImagePtr src, CImagePtr &dst, double strength)
{
  return src->unsharpMask(dst, strength);
}

CImagePtr
CImage::
unsharpMask(double strength)
{
  CImagePtr dst = CImageMgrInst->createImage();

  dst->setDataSize(size_);

  unsharpMask(dst, strength);

  return dst;
}

void
CImage::
unsharpMask(CImagePtr &dst, double strength)
{
  if (hasColormap()) {
    CImagePtr src = dup();

    src->convertToRGB();

    return src->unsharpMask(dst, strength);
  }

  //---

  // 5x5
  std::vector<double> kernel = {{ 0,  0,  1,  0, 0,
                                  0,  8, 21,  8, 0,
                                  1, 21, 59, 21, 1,
                                  0,  8, 21,  8, 0,
                                  0,  0,  1,  0, 0 }};

  convolve(dst, kernel);

  double gstrength = 1.0 - strength;

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y = wy1; y <= wy2; ++y) {
    for (int x = wx1; x <= wx2; ++x) {
      CRGBA rgba1, rgba2;

           getRGBAPixel(x, y, rgba1);
      dst->getRGBAPixel(x, y, rgba2);

      CRGBA rgba = rgba1*strength + rgba2*gstrength;

      rgba.clamp();

      dst->setRGBAPixel(x, y, rgba);
    }
  }
}

//---

CImagePtr
CImage::
sobel(bool feldman)
{
  CImagePtr dst = CImageMgrInst->createImage();

  dst->setDataSize(size_);

  sobel(dst, feldman);

  return dst;
}

void
CImage::
sobel(CImagePtr &dst, bool feldman)
{
  if (hasColormap()) {
    CImagePtr src = dup();

    src->convertToRGB();

    return src->sobel(dst, feldman);
  }

  //---

  CImagePtr dst1 = CImageMgrInst->createImage();
  CImagePtr dst2 = CImageMgrInst->createImage();

  dst1->setDataSize(size_);
  dst2->setDataSize(size_);

  std::vector<double> kernel1, kernel2;

  if (! feldman) {
    // sobel 3x3
    kernel1 = {{ -1,  0,  1, -2, 0, 2, -1, 0, 1 }};
    kernel2 = {{ -1, -2, -1,  0, 0, 0,  1, 2, 1 }};
  }
  else {
    // sobel-feldman 3x3
    kernel1 = {{ 3, 10,  3,  0, 0,   0, -3, -10, -3 }};
    kernel2 = {{ 3,  0, -3, 10, 0, -10,  3,   0, -3 }};
  }

  convolve(dst1, kernel1);
  convolve(dst2, kernel2);

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y = wy1; y <= wy2; ++y) {
    for (int x = wx1; x <= wx2; ++x) {
      CRGBA rgba1, rgba2;

      dst1->getRGBAPixel(x, y, rgba1);
      dst2->getRGBAPixel(x, y, rgba2);

      double r = std::min(hypot(rgba1.getRed  (), rgba2.getRed  ()), 1.0);
    //double g = std::min(hypot(rgba1.getGreen(), rgba2.getGreen()), 1.0);
    //double b = std::min(hypot(rgba1.getBlue (), rgba2.getBlue ()), 1.0);

      dst->setRGBAPixel(x, y, CRGBA(r, r, r));
    }
  }
}

//---

CImagePtr
CImage::
sobelGradient()
{
  CImagePtr dst = CImageMgrInst->createImage();

  dst->setDataSize(size_);

  sobelGradient(dst);

  return dst;
}

void
CImage::
sobelGradient(CImagePtr &dst)
{
  if (hasColormap()) {
    CImagePtr dst = dup();

    dst->convertToRGB();

    return sobelGradient(dst);
  }

  //---

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y = wy1; y <= wy2; ++y) {
    for (int x = wx1; x <= wx2; ++x) {
      double xgray, ygray, xf, yf;

      sobelPixelGradient(x, y, 1, 1, xgray, ygray, xf, yf);

      double l = hypot(xgray, ygray);
      double r = std::min(l, 1.0);

      dst->setRGBAPixel(x, y, CRGBA(r, r, r));
    }
  }
}

void
CImage::
sobelPixelGradient(int x, int y, int dx, int dy, double &xgray, double &ygray,
                   double &xf, double &yf)
{
  int w = getWidth ();
  int h = getHeight();

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  if (x < dx || x > w - dx - 1 || y < dy || y > h - dy - 1) {
    getGrayPixel(x, y, &xgray);

    ygray = xgray;

    xf = 1;
    yf = 1;

    return;
  }

  double gray1, gray2, gray3, gray4, gray5, gray6, gray7, gray8, gray9;

  bool left = (x < dx), right  = (x > w - dx - 1);
  bool top  = (y < dy), bottom = (y > h - dy - 1);

  if      (top && left) {
    getGrayPixel(x - dx, y - dy, &gray1);
    getGrayPixel(x     , y - dy, &gray2);
    getGrayPixel(x - dx, y     , &gray3);
    getGrayPixel(x     , y     , &gray4);

    xgray = -2*gray1 +2*gray2 -1*gray3 +1*gray4;
    ygray = -2*gray1 -1*gray2 +2*gray3 +1*gray4;

    xf = 2.0/(3*dx);
    yf = 2.0/(3*dy);
  }
  else if (top && right) {
    getGrayPixel(x     , y - dy, &gray1);
    getGrayPixel(x + dx, y - dy, &gray2);
    getGrayPixel(x     , y     , &gray3);
    getGrayPixel(x + dx, y     , &gray4);

    xgray = -2*gray1 +2*gray2 -1*gray3 +1*gray4;
    ygray = -1*gray1 -2*gray2 +1*gray3 +2*gray4;

    xf = 2.0/(3*dx);
    yf = 2.0/(3*dy);
  }
  else if (bottom && left) {
    getGrayPixel(x - dx, y     , &gray1);
    getGrayPixel(x     , y     , &gray2);
    getGrayPixel(x - dx, y + dy, &gray3);
    getGrayPixel(x     , y + dy, &gray4);

    xgray = -1*gray1 +1*gray2 -2*gray3 +2*gray4;
    ygray = -2*gray1 -1*gray2 +2*gray3 +1*gray4;

    xf = 2.0/(3*dx);
    yf = 2.0/(3*dy);
  }
  else if (bottom && right) {
    getGrayPixel(x     , y     , &gray1);
    getGrayPixel(x + dx, y     , &gray2);
    getGrayPixel(x     , y + dy, &gray3);
    getGrayPixel(x + dx, y + dy, &gray4);

    xgray = -1*gray1 +1*gray2 -2*gray3 +2*gray4;
    ygray = -1*gray1 -2*gray2 +1*gray3 +2*gray4;

    xf = 2.0/(3*dx);
    yf = 2.0/(3*dy);
  }
  else if (top) {
    getGrayPixel(x - dx, y - dy, &gray1);
    getGrayPixel(x     , y - dy, &gray2);
    getGrayPixel(x + dx, y - dy, &gray3);
    getGrayPixel(x - dx, y     , &gray4);
    getGrayPixel(x     , y     , &gray5);
    getGrayPixel(x + dx, y     , &gray6);

    xgray = -2*gray1          +2*gray3 -1*gray4          +1*gray6;
    ygray = -1*gray1 -2*gray2 -1*gray3 +1*gray4 +2*gray5 +1*gray6;

    xf = 1.0/(3*dx);
    yf = 1.0/(2*dy);
  }
  else if (left) {
    getGrayPixel(x - dx, y - dy, &gray1);
    getGrayPixel(x     , y - dy, &gray2);
    getGrayPixel(x - dx, y     , &gray3);
    getGrayPixel(x     , y     , &gray4);
    getGrayPixel(x - dx, y + dy, &gray5);
    getGrayPixel(x     , y + dy, &gray6);

    xgray = -1*gray1 +1*gray2 -2*gray3 +2*gray4 -1*gray5 +1*gray6;
    ygray = -2*gray1 -1*gray2                   +2*gray5 +1*gray6;

    xf = 1.0/(2*dx);
    yf = 1.0/(3*dy);
  }
  else if (right) {
    getGrayPixel(x     , y - dy, &gray1);
    getGrayPixel(x + dx, y - dy, &gray2);
    getGrayPixel(x     , y     , &gray3);
    getGrayPixel(x + dx, y     , &gray4);
    getGrayPixel(x     , y + dy, &gray5);
    getGrayPixel(x + dx, y + dy, &gray6);

    xgray = -1*gray1 +1*gray2 -2*gray3 +2*gray4 -1*gray5 +1*gray6;
    ygray = -1*gray1 -2*gray2                   +1*gray5 +2*gray6;

    xf = 1.0/(2*dx);
    yf = 1.0/(3*dy);
  }
  else if (bottom) {
    getGrayPixel(x - dx, y     , &gray1);
    getGrayPixel(x     , y     , &gray2);
    getGrayPixel(x + dx, y     , &gray3);
    getGrayPixel(x - dx, y + dy, &gray4);
    getGrayPixel(x     , y + dy, &gray5);
    getGrayPixel(x + dx, y + dy, &gray6);

    xgray = -1*gray1          +1*gray3 -2*gray4          +2*gray6;
    ygray = -1*gray1 -2*gray2 -1*gray3 +1*gray4 +2*gray5 +1*gray6;

    xf = 1.0/(3*dx);
    yf = 1.0/(2*dy);
  }
  else {
    getGrayPixel(x - dx, y - dy, &gray1);
    getGrayPixel(x     , y - dy, &gray2);
    getGrayPixel(x + dx, y - dy, &gray3);
    getGrayPixel(x - dx, y     , &gray4);
  //getGrayPixel(x     , y     , &gray5);
    getGrayPixel(x + dx, y     , &gray6);
    getGrayPixel(x - dx, y + dy, &gray7);
    getGrayPixel(x     , y + dy, &gray8);
    getGrayPixel(x + dx, y + dy, &gray9);

    xgray = -1*gray1          +1*gray3 -2*gray4          +2*gray6 -1*gray7          +1*gray9;
    ygray = -1*gray1 -2*gray2 -1*gray3                            +1*gray7 +2*gray8 +1*gray9;

    xf = 1.0/(4*dx);
    yf = 1.0/(4*dy);
  }
}

//---

void
CImage::
convolve(CImagePtr src, CImagePtr &dst, const std::vector<double> &kernel)
{
  return src->convolve(dst, kernel);
}

CImagePtr
CImage::
convolve(const std::vector<double> &kernel)
{
  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(size_);

  image->convolve(kernel);

  return image;
}

void
CImage::
convolve(CImagePtr &dst, const std::vector<double> &kernel)
{
  int size   = sqrt(kernel.size());
  int border = (size - 1)/2;

  double divisor = 0;

  for (const auto &k : kernel)
    divisor += k;

  if (divisor == 0)
    divisor = 1;

  //---

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  int y = wy1;

  for ( ; y < border; ++y) {
    int x = wx1;

    for ( ; x <= wx2 - border; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }
  }

  for ( ; y <= wy2 - border; ++y) {
    int x = wx1;

    for ( ; x < border; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }

    for ( ; x <= wx2 - border; ++x) {
      CRGBA sum;

      int k = 0;

      for (int yk = -border; yk <= border; ++yk) {
        for (int xk = -border; xk <= border; ++xk) {
          CRGBA rgba;

          getRGBAPixel(x + xk, y + yk, rgba);

          sum += rgba*kernel[k];

          ++k;
        }
      }

      sum /= divisor;

      sum.clamp();

      dst->setRGBAPixel(x, y, sum);
    }

    for ( ; x <= wx2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }
  }

  for ( ; y <= wy2; ++y) {
    int x = wx1;

    for ( ; x <= wx2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }
  }
}

bool
CImage::
gaussianBlur(CImagePtr src, CImagePtr &dst, double bx, double by, int nx, int ny)
{
  return src->gaussianBlur(dst, bx, by, nx, ny);
}

bool
CImage::
gaussianBlur(double bx, double by, int nx, int ny)
{
  CImagePtr dst = dup();

  if (! gaussianBlur(dst, bx, by, nx, ny))
    return false;

  replace(dst);

  return true;
}

bool
CImage::
gaussianBlur(CImagePtr &dst, double bx, double by, int nx, int ny)
{
  if (hasColormap()) {
    CImagePtr src = dup();

    src->convertToRGB();

    return src->gaussianBlur(dst, bx, by, nx, ny);
  }

  //---

  class CImageWrapper {
   public:
    CImageWrapper(CImage *image) :
     image_(image) {
    }

    void getPixelRange(int *x1, int *y1, int *x2, int *y2) const {
      *x1 = 0;
      *y1 = 0;
      *x2 = image_->getWidth () - 1;
      *y2 = image_->getHeight() - 1;
    }

    void getWindow(int *x1, int *y1, int *x2, int *y2) const {
      image_->getWindow(x1, y1, x2, y2);
    }

    void getRGBA(int x, int y, double *r, double *g, double *b, double *a) const {
      CRGBA rgba;

      image_->getRGBAPixel(x, y, rgba);

      *r = rgba.getRed  ();
      *g = rgba.getGreen();
      *b = rgba.getBlue ();
      *a = rgba.getAlpha();
    }

    void setRGBA(int x, int y, double r, double g, double b, double a) {
      image_->setRGBAPixel(x, y, CRGBA(r, g, b, a));
    }

   private:
    CImage *image_;
  };

  //---

  CGaussianBlur<CImageWrapper> blur;

  CImageWrapper wsrc(this);
  CImageWrapper wdst(dst.getPtr());

  blur.blur(wsrc, wdst, bx, by, nx, ny);

#if 0
  double minb = std::min(bx, by);

  if (minb <= 0)
    return false;

  //---

  // calc matrix size
  if (nx == 0) {
    nx = int(6*bx + 1);

    if (nx > 4) nx = 4;
  }

  if (ny == 0) {
    ny = int(6*by + 1);

    if (ny > 4) ny = 4;
  }

  int nx1 = -nx/2;
  int nx2 =  nx/2;
  int ny1 = -ny/2;
  int ny2 =  ny/2;

  nx = nx2 - nx1 + 1;
  ny = ny2 - ny1 + 1;

  //---

  // set matrix
  typedef std::vector<double> Reals;
  typedef std::vector<Reals>  RealsArray;

  RealsArray m;

  m.resize(nx);

  for (int i = 0; i < nx; ++i)
    m[i].resize(ny);

  double bxy  = bx*by;
  double bxy1 = 2*bxy;
  double bxy2 = 1.0/sqrt(M_PI*bxy1);

  double sm = 0.0;

  for (int i = 0, i1 = nx1; i < nx; ++i, ++i1) {
    int i2 = i1*i1;

    for (int j = 0, j1 = ny1; j < ny; ++j, ++j1) {
      int j2 = j1*j1;

      m[i][j] = bxy2*exp(-(i2 + j2)/bxy1);

      sm += m[i][j];
    }
  }

  //---

  // apply to image
  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y1 = wy1 + ny1, y2 = wy1, y3 = wy1 + ny2; y2 <= wy2; ++y1, ++y2, ++y3) {
    for (int x1 = wx1 + nx1, x2 = wx1, x3 = wx1 + nx2; x2 <= wx2; ++x1, ++x2, ++x3) {
      CRGBA  rgba;
      double a = 0.0;
      int    n = 0;

      for (int i = 0, x = x1; i < nx; ++i, ++x) {
        for (int j = 0, y = y1; j < ny; ++j, ++y) {
          if (! validPixel(x, y))
            continue;

          CRGBA rgba1;

          getRGBAPixel(x, y, rgba1);

          rgba += rgba1*m[i][j]/sm;
          a    += rgba1.getAlpha();

          ++n;
        }
      }

      rgba.clamp();

      rgba.setAlpha(a/n);

      dst->setRGBAPixel(x2, y2, rgba);
    }
  }
#endif

  return true;
}

class CTurbulenceUtil {
 private:
  static const int RAND_m = 2147483647; /* 2**31 - 1 */
  static const int RAND_a = 16807; /* 7**5; primitive root of m */
  static const int RAND_q = 127773; /* m / a */
  static const int RAND_r = 2836; /* m % a */

  static const int BSize   = 0x100;
  static const int BM      = 0xff;
  static const int PerlinN = 0x1000;
  static const int NP      = 12; /* 2^PerlinN */
  static const int NM      = 0xfff;

  //------

  int    latticeSelector[BSize + BSize + 2];
  double gradient[4][BSize + BSize + 2][2];

  //------

 public:
  CTurbulenceUtil(int seed) {
    init(seed);
  }

  /* Produces results in the range [1, 2**31 - 2].

     Algorithm is: r = (a * r) mod m
     where a = 16807 and m = 2**31 - 1 = 2147483647

     See [Park & Miller], CACM vol. 31 no. 10 p. 1195, Oct. 1988

     To test: the algorithm should produce the result 1043618065
     as the 10,000th generated number if the original seed is 1.
  */

 private:
  int initSeed(int lSeed) {
    if (lSeed <= 0)
      lSeed = -(lSeed % (RAND_m - 1)) + 1;

    if (lSeed > RAND_m - 1)
      lSeed = RAND_m - 1;

    return lSeed;
  }

  int random(int lSeed) {
    int result = RAND_a*(lSeed % RAND_q) - RAND_r*(lSeed / RAND_q);

    if (result <= 0)
      result += RAND_m;

    return result;
  }

  void init(int lSeed) {
    double s;

    lSeed = initSeed(lSeed);

    for (int k = 0; k < 4; k++) {
      for (int i = 0; i < BSize; i++) {
        latticeSelector[i] = i;

        for (int j = 0; j < 2; j++) {
          lSeed = random(lSeed);

          gradient[k][i][j] =
            double((lSeed) % (BSize + BSize) - BSize)/BSize;
        }

        s = sqrt(gradient[k][i][0]*gradient[k][i][0] +
                 gradient[k][i][1]*gradient[k][i][1]);

        gradient[k][i][0] /= s;
        gradient[k][i][1] /= s;
      }
    }

    int i = BSize;

    while (--i) {
      int k = latticeSelector[i];

      lSeed = random(lSeed);

      int j = lSeed % BSize;

      latticeSelector[i] = latticeSelector[j];
      latticeSelector[j] = k;
    }

    for (int i = 0; i < BSize + 2; i++) {
      latticeSelector[BSize + i] = latticeSelector[i];

      for (int k = 0; k < 4; k++)
        for (int j = 0; j < 2; j++)
          gradient[k][BSize + i][j] = gradient[k][i][j];
    }
  }

  double s_curve(double t) {
    return (t*t*(3.0 - 2.0*t));
  }

  double lerp(double t, double a, double b) {
    return (a + t*(b - a));
  }

  double noise2(int channelNum, double vec[2]) {
    double t = vec[0] + PerlinN;

    int bx0 = int(t)    & BM;
    int bx1 = (bx0 + 1) & BM;

    double rx0 = t - int(t);
    double rx1 = rx0 - 1.0;

    t = vec[1] + PerlinN;

    int by0 = int(t)    & BM;
    int by1 = (by0 + 1) & BM;

    double ry0 = t - int(t);
    double ry1 = ry0 - 1.0;

    int i = latticeSelector[bx0];
    int j = latticeSelector[bx1];

    int b00 = latticeSelector[i + by0];
    int b10 = latticeSelector[j + by0];
    int b01 = latticeSelector[i + by1];
    int b11 = latticeSelector[j + by1];

    double sx = s_curve(rx0);
    double sy = s_curve(ry0);

    double u, v, *q;

    q = gradient[channelNum][b00]; u = rx0*q[0] + ry0*q[1];
    q = gradient[channelNum][b10]; v = rx1*q[0] + ry0*q[1];

    double a = lerp(sx, u, v);

    q = gradient[channelNum][b01]; u = rx0*q[0] + ry1*q[1];
    q = gradient[channelNum][b11]; v = rx1*q[0] + ry1*q[1];

    double b = lerp(sx, u, v);

    return lerp(sy, a, b);
  }

  // Returns 'turbFunctionResult'

 public:
  double turbulence(int channelNum, double *point, double baseFreq, int numOctaves, bool fractal) {
    double vec[2];

    double sum       = 0;
    double frequency = baseFreq;

    for (int nOctave = 0; nOctave < numOctaves; nOctave++) {
      vec[0] = frequency*point[0];
      vec[1] = frequency*point[1];

      double amplitude = baseFreq/frequency;

      double sum1 = noise2(channelNum, vec)*amplitude;

      if (fractal)
        sum += sum1;
      else
        sum += fabs(sum1);

      frequency *= 2;
    }

    return sum;
  }
};

void
CImage::
turbulence(bool fractal, double baseFreq, int numOctaves, int seed)
{
  CTurbulenceUtil turbulence(seed);

  double r, g, b, a, point[2];

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y = wy1; y <= wy2; ++y) {
    for (int x = wx1; x <= wx2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      if (rgba.isTransparent())
        continue;

      //TODO: keep alpha ?
      //double a = rgba.getAlpha();

      point[0] = x;
      point[1] = y;

      r = turbulence.turbulence(0, point, baseFreq, numOctaves, fractal);
      g = turbulence.turbulence(1, point, baseFreq, numOctaves, fractal);
      b = turbulence.turbulence(2, point, baseFreq, numOctaves, fractal);
      a = turbulence.turbulence(3, point, baseFreq, numOctaves, fractal);

      if (fractal) {
        r = (r + 1.0) / 2.0;
        g = (g + 1.0) / 2.0;
        b = (b + 1.0) / 2.0;
        a = (a + 1.0) / 2.0;
      }

      r = std::min(std::max(r, 0.0), 1.0);
      g = std::min(std::max(g, 0.0), 1.0);
      b = std::min(std::max(b, 0.0), 1.0);
      a = std::min(std::max(a, 0.0), 1.0);

      setRGBAPixel(x, y, CRGBA(r, g, b, a));
    }
  }
}

CImagePtr
CImage::
displacementMap(CImagePtr dispImage, CColorComponent xcolor, CColorComponent ycolor, double scale)
{
  CImagePtr dst = CImageMgrInst->createImage();

  dst->setDataSize(size_);

  displacementMap(dispImage, xcolor, ycolor, scale, dst);

  return dst;
}

void
CImage::
displacementMap(CImagePtr dispImage, CColorComponent xcolor, CColorComponent ycolor,
                double scale, CImagePtr dst)
{
  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  for (int y = wy1; y <= wy2; ++y) {
    for (int x = wx1; x <= wx2; ++x) {
      CRGBA rgba1(0,0,0,0);

      // get displacement from dispImage color components
      if (dispImage->validPixel(x, y)) {
        CRGBA rgba2;

        dispImage->getRGBAPixel(x, y, rgba2);

        double rx = rgba2.getComponent(xcolor);
        double ry = rgba2.getComponent(ycolor);

        double x1 = x + scale*(rx - 0.5);
        double y1 = y + scale*(ry - 0.5);

        // TODO: interp pixel
        int ix1 = int(x1 + 0.5);
        int iy1 = int(y1 + 0.5);

        if (validPixel(ix1, iy1))
          getRGBAPixel(ix1, iy1, rgba1);
        else {
          //getRGBAPixel(x, y, rgba1);
        }
      }
      else {
        //getRGBAPixel(x, y, rgba1);
      }

      dst->setRGBAPixel(x, y, rgba1);
    }
  }
}
