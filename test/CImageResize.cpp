#include <std_c++.h>
#include <CFile/CFile.h>
#include <CFileUtil/CFileUtil.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc != 5) {
    cerr << "Usage: CImageResize " <<
            "<ifile> <ofile> <width> <height>" << endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  int width  = CStrUtil::toInteger(argv[3]);
  int height = CStrUtil::toInteger(argv[4]);

  CImageFileSrc src(*ifile);

  CImage::setResizeType(CIMAGE_RESIZE_BILINEAR);

  CImagePtr image = CImageMgrInst->createImage(src);

  CImagePtr image1;

  if      (width  == 0)
    image1 = image->resizeHeight(height);
  else if (height == 0)
    image1 = image->resizeWidth(width);
  else
    image1 = image->resize(width, height);

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  image1->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
