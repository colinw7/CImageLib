#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    std::cerr << "Usage: test_image_merge " <<
                 "<ifile1> <ifile2> <ofile>" << std::endl;
    exit(1);
  }

  CFile *ifile1 = new CFile(argv[1]);
  CFile *ifile2 = new CFile(argv[2]);
  CFile *ofile  = new CFile(argv[3]);

  CImageFileSrc src1(*ifile1);
  CImageFileSrc src2(*ifile2);

  CImagePtr image1 = CImageMgrInst->createImage(src1);
  CImagePtr image2 = CImageMgrInst->createImage(src2);

  if (! image1 || ! image2)
    exit(1);

  image2->convertToRGB();

  image2->setTransparentColor(CRGB(0, 0, 0));

  CImagePtr image = CImage::merge(image1, image2);

  CFileType type = CFileUtil::getImageType(argv[3]);

  image->write(ofile, type);

  delete ifile1;
  delete ifile2;
  delete ofile;

  exit(0);
}
