#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    std::cerr << "Usage: CImageIsometric " << "<ifile> <ofile> <width>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  int width1 = CStrUtil::toInteger(argv[3]);

  CImageFileSrc src(*ifile);

  CImage::setResizeType(CIMAGE_RESIZE_BILINEAR);

  CImagePtr image = CImageMgrInst->createImage(src);

  int width  = image->getWidth();
  int height = image->getHeight();

  CImagePtr image1 = CImageMgrInst->createImage();

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  double a = 30.0*M_PI/180.0;

  double c = cos(a);
  double s = sin(a);

  double hw = width1/2.0;

  double e  = hw/c;
  double hh = e*s;

  image1->setDataSize(width1, width1);

  image1->setRGBAData(CRGBA(0,0,0,0));

  double xs = e/width ;
  double ys = e/height;

  double xt = hw;
  double yt = (width1 - 2*hh)/2;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      CRGBA rgba;

      image->getRGBAPixel(x, y, rgba);

      double xx = x*xs;
      double yy = y*ys;

      double x1 = xt + xx*c - yy*c;
      double y1 = yt + xx*s + yy*s;

      image1->setRGBAPixel(int(x1 + 0.5), int(y1 + 0.5), rgba);
    }
  }

  image1->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
