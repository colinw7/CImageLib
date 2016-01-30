#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 8) {
    std::cerr << "Usage: test_image_turbulence " <<
                 "<ofile> w h fractal freq octaves seed" << std::endl;
    exit(1);
  }

  CFile ofile(argv[1]);

  int w, h;

  if (! CStrUtil::toInteger(argv[2], &w) ||
      ! CStrUtil::toInteger(argv[3], &h))
    exit(1);

  int fractal;

  if (! CStrUtil::toInteger(argv[4], &fractal))
    exit(1);

  double freq;

  if (! CStrUtil::toReal(argv[5], &freq))
    exit(1);

  int octaves, seed;

  if (! CStrUtil::toInteger(argv[6], &octaves) ||
      ! CStrUtil::toInteger(argv[7], &seed))
    exit(1);

  CImageNameSrc src("test_image_turbulence");

  CImagePtr image = CImageMgrInst->createImage(src);

  image->setDataSize(w, h);

  image->turbulence(fractal, freq, octaves, seed);

  CFileType type = CFileUtil::getImageTypeFromName(argv[1]);

  image->write(&ofile, type);

  exit(0);
}
