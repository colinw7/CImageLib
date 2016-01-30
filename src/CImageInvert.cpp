#include <CImageInvert.h>

CImagePtr
CImage::
inverted() const
{
  CImagePtr image = dup();

  image->invert();

  return image;
}

void
CImage::
invert()
{
  if (hasColormap()) {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        int ind = getColorIndexPixel(x, y);

        rgba = getColor(ind).invert();

        int ind1 = findColor(rgba);

        if (ind1 < 0) {
          ind1 = addColor(rgba);

          if (ind1 < 0) {
            setColor(ind, rgba);

            ind1 = ind;
          }
        }

        setColorIndexPixel(x, y, ind1);
      }
    }
  }
  else {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        getRGBAPixel(x, y, rgba);

        rgba.invert();

        setRGBAPixel(x, y, rgba);
      }
    }
  }
}
