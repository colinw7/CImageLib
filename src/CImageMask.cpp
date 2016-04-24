#include <CImageMask.h>

CImagePtr
CImage::
createMask() const
{
  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(size_);

  uint *p = image->data_;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++p) {
      if (! isTransparent(x, y))
        *p = 1;
    }
  }

  image->addColor(1, 1, 1);
  image->addColor(0, 0, 0);

  return image;
}

void
CImage::
alphaMask(CImagePtr mask, int xo, int yo)
{
  int mask_width  = mask->getWidth ();
  int mask_height = mask->getHeight();

  if (hasColormap()) {
    int num_colors = colors_.size();

    int transparent_ind = -1;

    for (int i = 0; i < num_colors; ++i)
      if (colors_[i].isTransparent()) {
        transparent_ind = i;
        break;
      }

    if (transparent_ind == -1) {
      if (num_colors < 256) {
        addColor(0, 0, 0, 0);

        transparent_ind = num_colors;
      }
    }

    if (transparent_ind == -1)
      transparent_ind = 255;

    for (int mask_y = 0, y = yo; mask_y < mask_height; ++mask_y, ++y) {
      for (int mask_x = 0, x = xo; mask_x < mask_width; ++mask_x, ++x) {
        if (! validPixel(x, y)) continue;

        if (mask->isTransparent(mask_x, mask_y))
          setColorIndexPixel(x, y, transparent_ind);
      }
    }
  }
  else {
    double r, g, b, a;

    for (int mask_y = 0, y = yo; mask_y < mask_height; ++mask_y, ++y) {
      for (int mask_x = 0, x = xo; mask_x < mask_width; ++mask_x, ++x) {
        if (! validPixel(x, y)) continue;

        getRGBAPixel(x, y, &r, &g, &b, &a);

        double a1 = mask->getAlpha(mask_x, mask_y);

        a = std::min(a*a1, 1.0);

        setRGBAPixel(x, y, r, g, b, a);
      }
    }
  }

  dataChanged();
}

void
CImage::
alphaMaskRGBA(CImagePtr mask, const CRGBA &rgba, int xo, int yo)
{
  int mask_width  = mask->getWidth ();
  int mask_height = mask->getHeight();

  if (hasColormap())
    convertToRGB();

  for (int mask_y = 0, y = yo; mask_y < mask_height; ++mask_y, ++y) {
    for (int mask_x = 0, x = xo; mask_x < mask_width; ++mask_x, ++x) {
      if (! validPixel(x, y)) continue;

      double a = mask->getAlpha(mask_x, mask_y);

      if (a < 0.5)
        setRGBAPixel(x, y, rgba);
    }
  }

  dataChanged();
}

CImagePtr
CImage::
createRGBAMask(const CRGBA &rgba)
{
  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(size_);

  //---

  // rgba contains r, g, b factors (default gray factors)
  double r, g, b, a;

  rgba.getRGBA(&r, &g, &b, &a);

  // normalize (default is already normalized)
  double sum = r + g + b;

  r /= sum;
  g /= sum;
  b /= sum;

  //---

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x) {
      double r1, g1, b1, a1;

      getRGBAPixel(x, y, &r1, &g1, &b1, &a1);

      a = a1*(r*r1 + g*g1 + b*b1);

      image->setRGBAPixel(x, y, r1, g1, b1, a);
    }
  }

  return image;
}

void
CImage::
clipOutside(int x1, int y1, int x2, int y2)
{
  CRGBA a(0,0,0,0);

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x) {
      if (y >= y1 && y <= y2 && x >= x1 && x <= x2)
        continue;

      setRGBAPixel(x, y, a);
    }
  }
}
