#include <CImageLib.h>
#include <CFile.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    std::cerr << "Usage: test_convert <file> <index_file> <rgb_file>" << std::endl;
    exit(1);
  }

  CFile *ifile  = new CFile(argv[1]);
  CFile *ofile1 = new CFile(argv[2]);
  CFile *ofile2 = new CFile(argv[3]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  image->convertToColorIndex();

  image->writeXPM(ofile1);

  image->convertToRGB();

  image->writeBMP(ofile2);

  delete ifile;
  delete ofile1;
  delete ofile2;

  exit(0);
}
