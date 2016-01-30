#include <CImageGrayScale.h>

CImagePtr
CImage::
grayScaled() const
{
  CImagePtr image = dup();

  image->grayScale();

  return image;
}

void
CImage::
grayScale()
{
  if (hasColormap()) {
    int num_colors = getNumColors();

    CRGBA rgba;

    for (int i = 0; i < num_colors; ++i) {
      rgba = getColor(i);

      rgba.toGray();

      setColor(i, rgba);
    }
  }
  else {
    int wx1, wy1, wx2, wy2;

    getWindow(&wx1, &wy1, &wx2, &wy2);

    CRGBA rgba;

    for (int y = wy1; y <= wy2; ++y) {
      for (int x = wx1; x <= wx2; ++x) {
        getRGBAPixel(x, y, rgba);

        rgba.toGray();

        setRGBAPixel (x, y, rgba);
      }
    }
  }

  dataChanged();
}
