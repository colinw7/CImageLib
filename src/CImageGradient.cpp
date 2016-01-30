#include <CImageGradient.h>
#include <CLinearGradient.h>
#include <CRadialGradient.h>

void
CImage::
linearGradient(const CLinearGradient &gradient)
{
  gradient.init(size_.width, size_.height);

  for (int y = 0; y < size_.height; ++y)
    for (int x = 0; x < size_.width; ++x)
      setRGBAPixel(x, y, gradient.getColor(x, y));
}

void
CImage::
radialGradient(const CRadialGradient &gradient)
{
  CRGBA rgba;

  gradient.init(size_.width, size_.height);

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x)
      setRGBAPixel(x, y, gradient.getColor(x, y));
  }
}
