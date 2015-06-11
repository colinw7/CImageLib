#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    cerr << "Usage: test_image_rotate <ifile> <ofile> <angle>" << endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  double angle = CStrUtil::toReal(argv[3]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  CImagePtr image1 = image->rotate(angle);

  image1->writeBMP(ofile);

  delete ifile;
  delete ofile;

  exit(0);
}
