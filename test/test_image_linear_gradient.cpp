#include <std_c++.h>
#include <CFile/CFile.h>
#include <CFileUtil/CFileUtil.h>
#include <CStrUtil/CStrUtil.h>
#include <CMath/CLinearGradient.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc < 11) {
    cerr << "Usage: test_image_linear_gradient " <<
            "<ofile> w h x1 y1 x2 y2 spread {color offset}" << endl;
    exit(1);
  }

  CLinearGradient gradient;

  CFile ofile(argv[1]);

  int w, h;

  if (! CStrUtil::toInteger(argv[2], &w) ||
      ! CStrUtil::toInteger(argv[3], &h))
    exit(1);

  double x1, y1, x2, y2;

  if (! CStrUtil::toReal(argv[4], &x1) ||
      ! CStrUtil::toReal(argv[5], &y1) ||
      ! CStrUtil::toReal(argv[6], &x2) ||
      ! CStrUtil::toReal(argv[7], &y2))
    exit(1);

  if      (strcmp(argv[8], "pad") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_PAD);
  else if (strcmp(argv[8], "repeat") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_REPEAT);
  else if (strcmp(argv[8], "reflect") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_REFLECT);

  gradient.setLine(x1, y1, x2, y2);

  int num_offsets = (argc - 9)/2;

  for (int i = 0; i < num_offsets; ++i) {
    CRGBA rgba(argv[2*i + 9]);

    double offset;

    if (! CStrUtil::toReal(argv[2*i + 10], &offset))
      exit(1);

    gradient.addStop(rgba, offset);
  }

  CImageNameSrc src("test_image_linear_gradient");

  CImagePtr image = CImageMgrInst->createImage(src);

  image->setDataSize(w, h);

  image->linearGradient(gradient);

  CFileType type = CFileUtil::getImageTypeFromName(argv[1]);

  image->write(&ofile, type);

  exit(0);
}
