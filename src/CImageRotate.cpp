#include "CImageLibI.h"
#include <cmath>

#define DEG_TO_RAD(d) (M_PI*(d)/180.0)

using std::min;
using std::max;

namespace {
  int Round(double x) {
    if (x >= 0) return int(x + 0.5);
    else        return int(x - 0.5);
  }
}

CImagePtr
CImage::
rotate(double angle)
{
  while (angle < 0)
    angle += 360;

  while (angle >= 360)
    angle -= 360;

  //-----

  CImagePtr image = CImageMgrInst->createImage();

  image->setType(getType());

  double c = cos(DEG_TO_RAD(angle));
  double s = sin(DEG_TO_RAD(angle));

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  int width  = right - left + 1;
  int height = top - bottom + 1;

  int width1  = (int) (fabs(width *c) + fabs(height*s));
  int height1 = (int) (fabs(height*c) + fabs(width *s));

  image->setDataSize(width1, height1);

  //-----

  if (hasColormap()) {
    int num_colors = getNumColors();

    for (int i = 0; i < num_colors; ++i)
      image->addColor(getColor(i));

    //-----

    if (isTransparent())
      image->setTransparentColor(getTransparentColor());
  }

  //-----

  double x_offset = 0;
  double y_offset = 0;

  if      (angle <= 90) {
    y_offset = (width - 1)*s;
  }
  else if (angle <= 180) {
    x_offset = -(width - 1)*c;
    y_offset = height1 - 1;
  }
  else if (angle <= 270) {
    x_offset = width1 - 1;
    y_offset = -(height - 1)*c;
  }
  else
    x_offset = -(height - 1)*s;

  if (hasColormap()) {
    for (int y = 0; y < height1; ++y) {
      for (int x = 0; x < width1; ++x) {
        int x1 = Round((x - x_offset)*c - (y - y_offset)*s) + left;
        int y1 = Round((x - x_offset)*s + (y - y_offset)*c) + bottom;

        x1 = min(max(x1, left  ), right);
        y1 = min(max(y1, bottom), top  );

        int ind = getColorIndexPixel(x1, y1);

        image->setColorIndexPixel(x, y, ind);
      }
    }
  }
  else {
    double r, g, b, a;

    for (int y = 0; y < height1; ++y) {
      for (int x = 0; x < width1; ++x) {
        int x1 = Round((x - x_offset)*c - (y - y_offset)*s) + left;
        int y1 = Round((x - x_offset)*s + (y - y_offset)*c) + bottom;

        x1 = min(max(x1, left  ), right);
        y1 = min(max(y1, bottom), top  );

        getRGBAPixel(x1, y1, &r, &g, &b, &a);

        image->setRGBAPixel(x, y, r, g, b, a);
      }
    }
  }

  return image;
}
