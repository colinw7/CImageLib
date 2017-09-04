#include <CImageResize.h>
#include <CMathRound.h>

#include <cstring>

CImageResizeType CImage::resize_type = CIMAGE_RESIZE_NEAREST;

CImageResizeType
CImage::
setResizeType(CImageResizeType type)
{
  std::swap(resize_type, type);

  return type;
}

CImagePtr
CImage::
scale(double s) const
{
  return scale(s, s);
}

CImagePtr
CImage::
scale(double xs, double ys) const
{
  return resize(int(xs*size_.width), int(ys*size_.height));
}

CImagePtr
CImage::
scaleKeepAspect(double s) const
{
  return scaleKeepAspect(s, s);
}

CImagePtr
CImage::
scaleKeepAspect(double xs, double ys) const
{
  return resizeKeepAspect(int(xs*size_.width), int(ys*size_.height));
}

CImagePtr
CImage::
resizeWidth(int width) const
{
  if (size_.width <= 0)
    return CImagePtr(this);

  int height = (width*size_.height)/size_.width;

  return resize(width, height);
}

CImagePtr
CImage::
resizeHeight(int height) const
{
  if (size_.height <= 0)
    return CImagePtr(this);

  int width = (height*size_.width)/size_.height;

  return resize(width, height);
}

CImagePtr
CImage::
resizeMax(int size) const
{
  double aspect = 1.0;

  if (size_.height > 0)
    aspect = (1.0*size_.width)/size_.height;

  if (size_.width > size_.height)
    return resize(size, (int) (size/aspect));
  else
    return resize((int) (size*aspect), size);
}

CImagePtr
CImage::
resizeMin(int size) const
{
  double aspect = 1.0;

  if (size_.height > 0)
    aspect = (1.0*size_.width)/size_.height;

  if (size_.width < size_.height)
    return resize(size, (int) (size/aspect));
  else
    return resize((int) (size*aspect), size);
}

CImagePtr
CImage::
resizeKeepAspect(const CISize2D &size) const
{
  return resizeKeepAspect(size.width, size.height);
}

CImagePtr
CImage::
resizeKeepAspect(int width, int height) const
{
  if (size_.width <= 0 || size_.height <= 0)
    return CImagePtr(this);

  double xfactor = (1.0*width )/size_.width ;
  double yfactor = (1.0*height)/size_.height;

  if (xfactor < yfactor)
    return resizeWidth(width);
  else
    return resizeHeight(height);
}

CImagePtr
CImage::
resize(const CISize2D &size) const
{
  return resize(size.width, size.height);
}

CImagePtr
CImage::
resize(int width, int height) const
{
  if (width <= 0 || height <= 0)
    return CImagePtr(this);

  CImagePtr image = dup();

  image->setType(getType());

  image->setDataSize(width, height);

  //------

  if (hasColormap()) {
    int num_colors = getNumColors();

    for (int i = 0; i < num_colors; ++i)
      image->addColor(getColor(i));

    //------

    if (isTransparent())
      image->setTransparentColor(getTransparentColor());
  }

  //------

  if (! hasColormap()) {
    if      (resize_type == CIMAGE_RESIZE_BILINEAR)
      reshapeBilinear(image);
    else if (resize_type == CIMAGE_RESIZE_AVERAGE)
      reshapeAverage(image);
    else if (resize_type == CIMAGE_RESIZE_NEAREST)
      reshapeNearest(image);
    else
      reshapeNearest(image);
  }
  else
    reshapeNearest(image);

  return image;
}

//---------------

bool
CImage::
reshapeWidth(int width)
{
  if (size_.width <= 0)
    return false;

  int height = (width*size_.height)/size_.width;

  return reshape(width, height);
}

bool
CImage::
reshapeHeight(int height)
{
  if (size_.height <= 0)
    return false;

  int width = (height*size_.width)/size_.height;

  return reshape(width, height);
}

bool
CImage::
reshapeMax(int size)
{
  if (size_.width <= 0 || size_.height <= 0)
    return false;

  double aspect = (1.0*size_.width)/size_.height;

  if (size_.width > size_.height)
    return reshape(size, (int) (size/aspect));
  else
    return reshape((int) (size*aspect), size);
}

bool
CImage::
reshapeMin(int size)
{
  if (size_.width <= 0 || size_.height <= 0)
    return false;

  double aspect = (1.0*size_.width)/size_.height;

  if (size_.width < size_.height)
    return reshape(size, (int) (size/aspect));
  else
    return reshape((int) (size*aspect), size);
}

bool
CImage::
reshapeKeepAspect(const CISize2D &size)
{
  return reshapeKeepAspect(size.width, size.height);
}

bool
CImage::
reshapeKeepAspect(int width, int height)
{
  if (size_.width <= 0 || size_.height <= 0)
    return false;

  double xfactor = (1.0*width )/size_.width ;
  double yfactor = (1.0*height)/size_.height;

  if (xfactor < yfactor)
    return reshapeWidth(width);
  else
    return reshapeHeight(height);
}

bool
CImage::
reshape(int width, int height)
{
  CImagePtr image = dup();

  image->setDataSize(width, height);

  //------

  if (! hasColormap()) {
    if      (resize_type == CIMAGE_RESIZE_BILINEAR)
      reshapeBilinear(image);
    else if (resize_type == CIMAGE_RESIZE_AVERAGE)
      reshapeAverage(image);
    else if (resize_type == CIMAGE_RESIZE_NEAREST)
      reshapeNearest(image);
    else
      reshapeNearest(image);
  }
  else
    reshapeNearest(image);

  replace(image);

  return true;
}

//---------------

void
CImage::
reshapeNearest(CImagePtr old_image, CImagePtr &new_image)
{
  old_image->reshapeNearest(new_image);
}

void
CImage::
reshapeNearest(CImagePtr &new_image) const
{
  int width1  = getWidth ();
  int height1 = getHeight();

  if (width1 <= 0 || height1 <= 0)
    return;

  int width2  = new_image->getWidth ();
  int height2 = new_image->getHeight();

  if (width2 <= 0 || height2 <= 0)
    return;

  double dx = (1.0*width1 )/width2 ;
  double dy = (1.0*height1)/height2;

  double y1 = 0.0;

  if (! hasColormap()) {
    for (int y = 0; y < height2; ++y, y1 += dy) {
      double x1 = 0.0;

      for (int x = 0; x < width2; ++x, x1 += dx) {
        int x2 = std::min(std::max(int(x1), 0), width1  - 1);
        int y2 = std::min(std::max(int(y1), 0), height1 - 1);

        double r, g, b, a;

        getRGBAPixel(x2, y2, &r, &g, &b, &a);

        new_image->setRGBAPixel(x, y, r, g, b, a);
      }
    }
  }
  else {
    for (int y = 0; y < height2; ++y, y1 += dy) {
      double x1 = 0.0;

      for (int x = 0; x < width2; ++x, x1 += dx) {
        int x2 = std::min(std::max(int(x1), 0), width1  - 1);
        int y2 = std::min(std::max(int(y1), 0), height1 - 1);

        int pixel = getColorIndexPixel(x2, y2);

        new_image->setColorIndexPixel(x, y, pixel);
      }
    }
  }
}

void
CImage::
reshapeAverage(CImagePtr old_image, CImagePtr &new_image)
{
  old_image->reshapeAverage(new_image);
}

void
CImage::
reshapeAverage(CImagePtr &new_image) const
{
  if (hasColormap())
    return reshapeNearest(new_image);

  int width1  = getWidth ();
  int height1 = getHeight();

  int width2  = new_image->getWidth ();
  int height2 = new_image->getHeight();

  if (width2 <= 0 || height2 <= 0)
    return;

  double dx = (1.0*width1 )/width2 ;
  double dy = (1.0*height1)/height2;

  int xx1, yy1, xx2, yy2;

  double y1 = 0.0;
  double y2 = dy;

  for (int y = 0; y < height2; ++y, y1 = y2, y2 += dy) {
    yy1 = std::min(std::max(CMathRound::Round(y1), 0), height1 - 1);
    yy2 = std::min(std::max(CMathRound::Round(y2), 0), height1 - 1);

    double x1 = 0.0;
    double x2 = dx;

    for (int x = 0; x < width2; ++x, x1 = x2, x2 += dx) {
      xx1 = std::min(std::max(CMathRound::Round(x1), 0), width1 - 1);
      xx2 = std::min(std::max(CMathRound::Round(x2), 0), width1 - 1);

      double r = 0.0, g = 0.0, b = 0.0, a = 0.0;

      for (int yy = yy1; yy <= yy2; ++yy) {
        for (int xx = xx1; xx <= xx2; ++xx) {
          double r1, g1, b1, a1;

          getRGBAPixel(xx, yy, &r1, &g1, &b1, &a1);

          r += r1;
          g += g1;
          b += b1;
          a += a1;
        }
      }

      int n = (xx2 - xx1 + 1)*(yy2 - yy1 + 1);

      r /= n;
      g /= n;
      b /= n;
      a /= n;

      new_image->setRGBAPixel(x, y, r, g, b, a);
    }
  }
}

#if 0
CRGBA
CImage::
getAverageRGBAPixel(double x1, double x2, double y1, double y1) const
{
  double r = 0.0, g = 0.0, b = 0.0, a = 0.0;

  int width1  = getWidth ();
  int height1 = getHeight();

  int yy1 = std::min(std::max(CMathRound::Round(y1), 0), height1 - 1);
  int yy2 = std::min(std::max(CMathRound::Round(y2), 0), height1 - 1);
  int xx1 = std::min(std::max(CMathRound::Round(x1), 0), width1 - 1);
  int xx2 = std::min(std::max(CMathRound::Round(x2), 0), width1 - 1);

  for (int yy = yy1; yy <= yy2; ++yy) {
    for (int xx = xx1; xx <= xx2; ++xx) {
      double r1, g1, b1, a1;

      getRGBAPixel(xx, yy, &r1, &g1, &b1, &a1);

      r += r1;
      g += g1;
      b += b1;
      a += a1;
    }
  }

  int n = (xx2 - xx1 + 1)*(yy2 - yy1 + 1);

  r /= n;
  g /= n;
  b /= n;
  a /= n;

  return CRGBA(r, g, b, a);
}
#endif

void
CImage::
reshapeBilinear(CImagePtr old_image, CImagePtr &new_image)
{
  return old_image->reshapeBilinear(new_image);
}

void
CImage::
reshapeBilinear(CImagePtr &new_image) const
{
  if (hasColormap()) {
    reshapeNearest(new_image);

    return;
  }

  int width1  = getWidth ();
  int height1 = getHeight();

  int width2  = new_image->getWidth ();
  int height2 = new_image->getHeight();

  if (width2 <= 0 || height2 <= 0)
    return;

  double ix = (1.0*width1 )/width2 ;
  double iy = (1.0*height1)/height2;

  double yy = 0.0;

  for (int y = 0; y < height2; ++y, yy += iy) {
    double xx = 0.0;

    int y1 = CMathRound::RoundDown(yy);
    int y2 = CMathRound::RoundUp  (yy);

    if (y2 >= height1) y2 = height1 - 1;

    for (int x = 0; x < width2; ++x, xx += ix) {
      int x1 = CMathRound::RoundDown(xx);
      int x2 = CMathRound::RoundUp  (xx);

      if (x2 >= width1) x2 = width1 - 1;

      CRGBA c = getBilinearRGBAPixel(xx, x1, x2, yy, y1, y2);

      new_image->setRGBAPixel(x, y, c);
    }
  }
}

CRGBA
CImage::
getBilinearRGBAPixel(double xx, double yy) const
{
  int width1  = getWidth ();
  int height1 = getHeight();

  int x1 = CMathRound::RoundDown(xx);
  int x2 = CMathRound::RoundUp  (xx);

  if (x1 <  0     ) x1 = 0;
  if (x2 >= width1) x2 = width1 - 1;

  int y1 = CMathRound::RoundDown(yy);
  int y2 = CMathRound::RoundUp  (yy);

  if (y1 <  0      ) y2 = 0;
  if (y2 >= height1) y2 = height1 - 1;

  return getBilinearRGBAPixel(xx, x1, x2, yy, y1, y2);
}

CRGBA
CImage::
getBilinearRGBAPixel(double xx, int x1, int x2, double yy, int y1, int y2) const
{
  if (x1 != x2 || y1 != y2) {
    double r1, g1, b1, a1;
    double r2, g2, b2, a2;
    double r3, g3, b3, a3;
    double r4, g4, b4, a4;

    getRGBAPixel(x1, y2, &r1, &g1, &b1, &a1);
    getRGBAPixel(x2, y2, &r2, &g2, &b2, &a2);
    getRGBAPixel(x1, y1, &r3, &g3, &b3, &a3);
    getRGBAPixel(x2, y1, &r4, &g4, &b4, &a4);

    double dx  = x2 - xx;
    double dx1 = 1.0 - dx;
    double dy  = yy - y1;
    double dy1 = 1.0 - dy;

    double r12 = dx*r1 + dx1*r2;
    double r34 = dx*r3 + dx1*r4;

    double r = (dy*r12 + dy1*r34);

    double g12 = dx*g1 + dx1*g2;
    double g34 = dx*g3 + dx1*g4;

    double g = (dy*g12 + dy1*g34);

    double b12 = dx*b1 + dx1*b2;
    double b34 = dx*b3 + dx1*b4;

    double b = (dy*b12 + dy1*b34);

    double a12 = dx*a1 + dx1*a2;
    double a34 = dx*a3 + dx1*a4;

    double a = (dy*a12 + dy1*a34);

    return CRGBA(r, g, b, a);
  }
  else {
    double r, g, b, a;

    getRGBAPixel(x1, y1, &r, &g, &b, &a);

    return CRGBA(r, g, b, a);
  }
}

//-----------------

void
CImage::
sampleNearest(double x, double y, CRGBA &rgba) const
{
  int px = CMathRound::Round(x*(getWidth () - 1));
  int py = CMathRound::Round(y*(getHeight() - 1));

  getRGBAPixel(px, py, rgba);
}

void
CImage::
sampleBilinear(double x, double y, CRGBA &rgba) const
{
  double x1 = x*(getWidth () - 1);
  double y1 = y*(getHeight() - 1);

  int px1 = CMathRound::RoundDown(x1);
  int py1 = CMathRound::RoundDown(y1);
  int px2 = CMathRound::RoundUp  (x1);
  int py2 = CMathRound::RoundUp  (y1);

  if (px1 != px2 || py1 != py2) {
    CRGBA rgba1, rgba2, rgba3, rgba4;

    getRGBAPixel(px1, py2, rgba1);
    getRGBAPixel(px2, py2, rgba2);
    getRGBAPixel(px1, py1, rgba3);
    getRGBAPixel(px2, py1, rgba4);

    double dx  = px2 - x1;
    double dx1 = 1.0 - dx;
    double dy  = y1 - py2;
    double dy1 = 1.0 - dy;

    CRGBA rgba12 = dx*rgba1 + dx1*rgba2;
    CRGBA rgba34 = dx*rgba3 + dx1*rgba4;

    rgba = (dy*rgba12 + dy1*rgba34);
  }
  else
    getRGBAPixel(px1, py1, rgba);
}
