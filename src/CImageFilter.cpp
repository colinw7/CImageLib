#include <CImageFilter.h>
#include <CGaussianBlur.h>
#include <CTurbulenceUtil.h>
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

  convolve(dst, 5, 5, kernel);

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

  convolve(dst1, 3, 3, kernel1);
  convolve(dst2, 3, 3, kernel2);

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
  int size = sqrt(kernel.size());

  return convolve(src, dst, size, size, kernel);
}

CImagePtr
CImage::
convolve(const std::vector<double> &kernel)
{
  int size = sqrt(kernel.size());

  return convolve(size, size, kernel);
}

void
CImage::
convolve(CImagePtr &dst, const std::vector<double> &kernel)
{
  int size = sqrt(kernel.size());

  return convolve(dst, size, size, kernel);
}

void
CImage::
convolve(CImagePtr src, CImagePtr &dst, int xsize, int ysize, const std::vector<double> &kernel)
{
  return src->convolve(dst, xsize, ysize, kernel);
}

CImagePtr
CImage::
convolve(int xsize, int ysize, const std::vector<double> &kernel)
{
  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(size_);

  image->convolve(xsize, ysize, kernel);

  return image;
}

void
CImage::
convolve(CImagePtr &dst, int xsize, int ysize, const std::vector<double> &kernel)
{
  CImageConvolveData data;

  data.xsize = xsize;
  data.ysize = ysize;
  data.kernel = kernel;

  convolve(dst, data);
}

void
CImage::
convolve(CImagePtr &dst, const CImageConvolveData &data)
{
  int xsize = data.xsize;
  int ysize = data.ysize;

  if (xsize < 0)
    xsize = sqrt(data.kernel.size());

  if (ysize < 0)
    ysize = sqrt(data.kernel.size());

  //---

  int xborder = (xsize - 1)/2;
  int yborder = (ysize - 1)/2;

  double divisor = data.divisor;

  if (divisor < 0) {
    divisor = 0;

    for (const auto &k : data.kernel)
      divisor += k;

    if (divisor == 0)
      divisor = 1;
  }

  //---

  int wx1, wy1, wx2, wy2;

  getWindow(&wx1, &wy1, &wx2, &wy2);

  int y = wy1;

  for ( ; y < yborder; ++y) {
    int x = wx1;

    for ( ; x <= wx2 - xborder; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }
  }

  for ( ; y <= wy2 - yborder; ++y) {
    int x = wx1;

    for ( ; x < xborder; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      dst->setRGBAPixel(x, y, rgba);
    }

    for ( ; x <= wx2 - xborder; ++x) {
      CRGBA sum;

      int k = 0;

      for (int yk = -yborder; yk <= yborder; ++yk) {
        for (int xk = -xborder; xk <= xborder; ++xk) {
          CRGBA rgba;

          getRGBAPixel(x + xk, y + yk, rgba);

          sum += rgba*data.kernel[k];

          ++k;
        }
      }

      sum /= divisor;

      sum.clamp();

      if (data.preserveAlpha) {
        CRGBA rgba;

        getRGBAPixel(x, y, rgba);

        sum.setAlpha(rgba.getAlpha());
      }

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
  return gaussianBlurExec(dst, bx, by, nx, ny);
}

bool
CImage::
gaussianBlurExec(CImagePtr &dst, double bx, double by, int nx, int ny)
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

void
CImage::
turbulence(bool fractal, double baseFreq, int numOctaves, int seed)
{
  turbulence(fractal, baseFreq, baseFreq, numOctaves, seed);
}

void
CImage::
turbulence(bool fractal, double baseFreqX, double baseFreqY, int numOctaves, int seed)
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

      r = turbulence.turbulence(0, point, baseFreqX, baseFreqY, numOctaves, fractal);
      g = turbulence.turbulence(1, point, baseFreqX, baseFreqY, numOctaves, fractal);
      b = turbulence.turbulence(2, point, baseFreqX, baseFreqY, numOctaves, fractal);
      a = turbulence.turbulence(3, point, baseFreqX, baseFreqY, numOctaves, fractal);

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
displacementMap(CImagePtr dispImage, CRGBAComponent xcolor, CRGBAComponent ycolor, double scale)
{
  CImagePtr dst = CImageMgrInst->createImage();

  dst->setDataSize(size_);

  displacementMap(dispImage, xcolor, ycolor, scale, dst);

  return dst;
}

void
CImage::
displacementMap(CImagePtr dispImage, CRGBAComponent xcolor, CRGBAComponent ycolor,
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
