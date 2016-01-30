#include <CImageLib.h>
#include <CRadialGradient.h>
#include <CRGBName.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <cstring>

int
main(int argc, char **argv)
{
  if (argc < 12) {
    std::cerr << "Usage: test_image_radial_gradient " <<
                 "<ofile> w h xc yc r fx fy spread {color offset}" << std::endl;
    exit(1);
  }

  CRadialGradient gradient;

  CFile ofile(argv[1]);

  int w, h;

  if (! CStrUtil::toInteger(argv[2], &w) ||
      ! CStrUtil::toInteger(argv[3], &h))
    exit(1);

  double xc, yc, r;

  if (! CStrUtil::toReal(argv[4], &xc) ||
      ! CStrUtil::toReal(argv[5], &yc) ||
      ! CStrUtil::toReal(argv[6], &r ))
    exit(1);

  double fx, fy;

  if (! CStrUtil::toReal(argv[7], &fx) ||
      ! CStrUtil::toReal(argv[8], &fy))
    exit(1);

  gradient.setCenter(xc, yc);
  gradient.setRadius(r);
  gradient.setFocus (fx, fy);

  if      (strcmp(argv[9], "pad") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_PAD);
  else if (strcmp(argv[9], "repeat") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_REPEAT);
  else if (strcmp(argv[9], "reflect") == 0)
    gradient.setSpread(CGRADIENT_SPREAD_REFLECT);

  int num_offsets = (argc - 10)/2;

  for (int i = 0; i < num_offsets; ++i) {
    CRGBA rgba = CRGBName::toRGBA(argv[2*i + 10]);

    double offset;

    if (! CStrUtil::toReal(argv[2*i + 11], &offset))
      exit(1);

    gradient.addStop(rgba, offset);
  }

  CImageNameSrc src("test_image_radial_gradient");

  CImagePtr image = CImageMgrInst->createImage(src);

  image->setDataSize(w, h);

  image->radialGradient(gradient);

  CFileType type = CFileUtil::getImageTypeFromName(argv[1]);

  image->write(&ofile, type);

  exit(0);
}
