#include <CImageMerge.h>

CImagePtr
CImage::
merge(CImagePtr image1, CImagePtr image2)
{
  CImagePtr timage1 = image1;
  CImagePtr timage2 = image2;

  if (image1->hasColormap()) {
    timage1 = image1->dup();

    timage1->convertToRGB();
  }

  if (image2->hasColormap()) {
    timage2 = image2->dup();

    timage2->convertToRGB();
  }

  CImagePtr image = CImageMgrInst->createImage();

  image->setType(timage1->getType());

  auto width1  = timage1->getWidth ();
  auto height1 = timage1->getHeight();

  auto width2  = timage2->getWidth ();
  auto height2 = timage2->getHeight();

  auto min_width  = std::min(width1 , width2 );
  auto min_height = std::min(height1, height2);
  auto max_width  = std::max(width1 , width2 );
  auto max_height = std::max(height1, height2);

  image->setDataSize(int(max_width), int(max_height));

  //------

  auto dxl1 = (max_width  - width1 )/2;
  auto dyt1 = (max_height - height1)/2;
  auto dxl2 = (max_width  - width2 )/2;
  auto dyt2 = (max_height - height2)/2;

  auto dxr1 = max_width  - width1  - dxl1;
  auto dyb1 = max_height - height1 - dyt1;
  auto dxr2 = max_width  - width2  - dxl2;
  auto dyb2 = max_height - height2 - dyt2;

  double r1, g1, b1, a1;
  double r2, g2, b2, a2;

  int y = 0;

  for (int i = 0; i < int(dyt2); ++i, ++y) {
    for (int x = 0; x < int(width1); ++x) {
      timage1->getRGBAPixel(x, y, &r1, &g1, &b1, &a1);

      image->setRGBAPixel(x, y, r1, g1, b1, a1);
    }
  }

  for (int i = 0; i < int(dyt1); ++i, ++y) {
    for (int x = 0; x < int(width2); ++x) {
      timage2->getRGBAPixel(x, y, &r2, &g2, &b2, &a2);

      image->setRGBAPixel(x, y, r2*a2, g2*a2, b2*a2, a2);
    }
  }

  for (int i = 0; i < int(min_height); ++i, ++y) {
    int x = 0;

    for (int j = 0; j < int(dxl2); ++j, ++x) {
      timage1->getRGBAPixel(x, y - int(dyt1), &r1, &g1, &b1, &a1);

      image->setRGBAPixel(x, y, r1, g1, b1, a1);
    }

    for (int j = 0; j < int(dxl1); ++j, ++x) {
      timage2->getRGBAPixel(x, y - int(dyt2), &r2, &g2, &b2, &a2);

      image->setRGBAPixel(x, y, r2*a2, g2*a2, b2*a2, a2);
    }

    for (int j = 0; j < int(min_width); ++j, ++x) {
      timage1->getRGBAPixel(x - int(dxl1), y - int(dyt1), &r1, &g1, &b1, &a1);
      timage2->getRGBAPixel(x - int(dxl2), y - int(dyt2), &r2, &g2, &b2, &a2);

      double a11 = std::min(1 - a2, a1);

      image->setRGBAPixel(x, y,
                          r2*a2 + a11*r1,
                          g2*a2 + a11*g1,
                          b2*a2 + a11*b1,
                          a11 + a2);
    }

    for (int j = 0; j < int(dxr2); ++j, ++x) {
      timage1->getRGBAPixel(x, y - int(dyt1), &r1, &g1, &b1, &a1);

      image->setRGBAPixel(x, y, r1, g1, b1, a1);
    }

    for (int j = 0; j < int(dxr1); ++j, ++x) {
      timage2->getRGBAPixel(x, y - int(dyt2), &r2, &g2, &b2, &a2);

      image->setRGBAPixel(x, y, r2*a2, g2*a2, b2*a2, a2);
    }
  }

  for (int i = 0; i < int(dyb2); ++i, ++y) {
    for (int x = 0; x < int(width1); ++x) {
      timage1->getRGBAPixel(x, y, &r1, &g1, &b1, &a1);

      image->setRGBAPixel(x, y, r1, g1, b1, a1);
    }
  }

  for (int i = 0; i < int(dyb1); ++i, ++y) {
    for (int x = 0; x < int(width2); ++x) {
      timage2->getRGBAPixel(x, y, &r2, &g2, &b2, &a2);

      image->setRGBAPixel(x, y, r2*a2, g2*a2, b2*a2, a2);
    }
  }

  //------

  return image;
}
