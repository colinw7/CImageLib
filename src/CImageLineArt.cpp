#include <CImageLineArt.h>

#define CIMAGE_LINE_ART_TOLERANCE 0.5

void
CImage::
lineArt(double tolerance)
{
  double r, g, b, a;

  if (hasColormap()) {
    int num_colors = getNumColors();

    CRGBA rgba;

    for (int i = 0; i < num_colors; ++i) {
      rgba = getColor(uint(i));

      r = rgba.getRed  ();
      g = rgba.getGreen();
      b = rgba.getBlue ();

      if (r > tolerance || g > tolerance || b > tolerance)
        rgba.setRGBA(1.0, 1.0, 1.0);
      else
        rgba.setRGBA(0.0, 0.0, 0.0);

      setColor(uint(i), rgba);
    }
  }
  else {
    auto width  = getWidth ();
    auto height = getHeight();

    int i = 0;

    for (int y = 0; y < int(height); ++y) {
      for (int x = 0; x < int(width); ++x, ++i) {
        getRGBAPixel(i, &r, &g, &b, &a);

        if (r > tolerance || g > tolerance || b > tolerance)
          setRGBAPixel(i, 1.0, 1.0, 1.0);
        else
          setRGBAPixel(i, 0.0, 0.0, 0.0);
      }
    }
  }
}
