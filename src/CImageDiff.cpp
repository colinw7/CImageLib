#include <CImageDiff.h>

bool
CImage::
diffValue(const CImagePtr &image, double &d)
{
  d = 0.0;

  int wx11, wy11, wx21, wy21;
  int wx12, wy12, wx22, wy22;

         getWindow(&wx11, &wy11, &wx21, &wy21);
  image->getWindow(&wx12, &wy12, &wx22, &wy22);

  if (wx21 - wx11 != wx22 - wx12)
    return false;

  if (wy21 - wy11 != wy22 - wy12)
    return false;

  int w = wx21 - wx11 + 1;
  int h = wy21 - wy11 + 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba1, rgba2;

             getRGBAPixel(wx11 + x, wy11 + y, rgba1);
      image->getRGBAPixel(wx12 + x, wy12 + y, rgba2);

      double dr = std::abs(rgba1.getRed  () - rgba2.getRed  ());
      double dg = std::abs(rgba1.getGreen() - rgba2.getGreen());
      double db = std::abs(rgba1.getBlue () - rgba2.getBlue ());
      double da = std::abs(rgba1.getAlpha() - rgba2.getAlpha());

      d += (dr + dg + db + da);
    }
  }

  return true;
}

bool
CImage::
diffImage(const CImagePtr &image, CImagePtr &dest)
{
  dest = image->dup();

  int wx11, wy11, wx21, wy21;
  int wx12, wy12, wx22, wy22;

         getWindow(&wx11, &wy11, &wx21, &wy21);
  image->getWindow(&wx12, &wy12, &wx22, &wy22);

  if (wx21 - wx11 != wx22 - wx12)
    return false;

  if (wy21 - wy11 != wy22 - wy12)
    return false;

  int w = wx21 - wx11 + 1;
  int h = wy21 - wy11 + 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba1, rgba2;

             getRGBAPixel(wx11 + x, wy11 + y, rgba1);
      image->getRGBAPixel(wx12 + x, wy12 + y, rgba2);

      rgba1 = rgba1.normalized();
      rgba2 = rgba2.normalized();

      double dr = std::abs(rgba1.getRed  () - rgba2.getRed  ());
      double dg = std::abs(rgba1.getGreen() - rgba2.getGreen());
      double db = std::abs(rgba1.getBlue () - rgba2.getBlue ());

      if (dr > 1E-3 || dg > 1E-3 || db > 1E-3)
        dest->setRGBAPixel(wx11 + x, wy11 + y, CRGBA(dr, dg, db));
      else
        dest->setRGBAPixel(wx11 + x, wy11 + y, CRGBA(0, 0, 0, 0));
    }
  }

  return true;
}
