#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 5) {
    std::cerr << "Usage: test_image_filter " <<
                 "<ifile> <ofile> bx by [nx] [ny]" << std::endl;
    exit(1);
  }

  CFile ifile(argv[1]);
  CFile ofile(argv[2]);

  double bx, by;

  if (! CStrUtil::toReal(argv[3], &bx) ||
      ! CStrUtil::toReal(argv[4], &by))
    exit(1);

  int nx = 0;
  int ny = 0;

  if (argc == 7) {
    if (! CStrUtil::toInteger(argv[5], &nx) ||
        ! CStrUtil::toInteger(argv[6], &ny))
      exit(1);
  }

  CImageFileSrc src(ifile);

  CImagePtr src_image = CImageMgrInst->createImage(src);

  CImageNameSrc src1("test_image_filter");

  CImagePtr dest_image = CImageMgrInst->createImage(src1);

  dest_image->setDataSize(src_image->getWidth(), src_image->getHeight());

  CImage::gaussianBlur(src_image, dest_image, bx, by, nx, ny);

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  dest_image->write(&ofile, type);

  exit(0);
}
