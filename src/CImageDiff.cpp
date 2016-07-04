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
diffImage(const CImagePtr &image, CImagePtr &dest, CImageDiffData &diffData)
{
  diffData.diff = 0.0;

  dest = image->dup();

  int wx11, wy11, wx21, wy21;
  int wx12, wy12, wx22, wy22;

         getWindow(&wx11, &wy11, &wx21, &wy21);
  image->getWindow(&wx12, &wy12, &wx22, &wy22);

  bool rc = true;

  if (wx21 - wx11 != wx22 - wx12)
    rc = false;

  if (wy21 - wy11 != wy22 - wy12)
    rc = false;

  bool hasBg = (diffData.bg.getAlpha() > 0);

  int w = std::min(wx21 - wx11 + 1, wx22 - wx12 + 1);
  int h = std::min(wy21 - wy11 + 1, wy22 - wy12 + 1);

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba1, rgba2;

             getRGBAPixel(wx11 + x, wy11 + y, rgba1);
      image->getRGBAPixel(wx12 + x, wy12 + y, rgba2);

      if (hasBg) {
        rgba1 = diffData.bg.combined(rgba1);
        rgba2 = diffData.bg.combined(rgba2);
      }
      else {
        rgba1 = rgba1.normalized();
        rgba2 = rgba2.normalized();
      }

      double dr = std::fabs(rgba1.getRed  () - rgba2.getRed  ());
      double dg = std::fabs(rgba1.getGreen() - rgba2.getGreen());
      double db = std::fabs(rgba1.getBlue () - rgba2.getBlue ());
      double da = std::abs(rgba1.getAlpha() - rgba2.getAlpha());

      diffData.diff += (dr + dg + db + da);

      if (dr > 1E-3 || dg > 1E-3 || db > 1E-3) {
        if (diffData.grayScale) {
          double g = (dr + dg + db)/3;

          dest->setRGBAPixel(wx11 + x, wy11 + y, CRGBA(0, 0, 0, g));
        }
        else
          dest->setRGBAPixel(wx11 + x, wy11 + y, diffData.fg);
      }
      else
        dest->setRGBAPixel(wx11 + x, wy11 + y, CRGBA(0, 0, 0, 0));
    }
  }

  return rc;
}
