#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 4) {
    std::cerr << "Usage: test_image_rgb_mask " <<
                 "<ifile> <mfile> <ofile> [r] [g] [b]" << std::endl;
    exit(1);
  }

  CFile ifile(argv[1]);
  CFile mfile(argv[2]);
  CFile ofile(argv[3]);

  double r, g, b;

  if (argc >= 5 && ! CStrUtil::toReal(argv[4], &r)) exit(1);
  if (argc >= 6 && ! CStrUtil::toReal(argv[5], &g)) exit(1);
  if (argc >= 7 && ! CStrUtil::toReal(argv[6], &b)) exit(1);

  CImageFileSrc src1(ifile);

  CImagePtr src_image = CImageMgrInst->createImage(src1);

  CImageFileSrc src2(mfile);

  CImagePtr merge_image = CImageMgrInst->createImage(src2);

  CImagePtr merge_image1;

  if (argc >= 5) {
    CRGBA rgba(r, g, b);

    merge_image1 = merge_image->createRGBAMask(rgba);
  }
  else
    merge_image1 = merge_image->createRGBAMask();

  src_image->combineAlpha(merge_image1);

  CFileType type = CFileUtil::getImageTypeFromName(argv[3]);

  src_image->write(&ofile, type);

  exit(0);
}
