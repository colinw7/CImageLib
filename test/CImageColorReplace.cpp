#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <CRGBName.h>
#include <cstring>

int
main(int argc, char **argv)
{
  bool debug = false;

  std::string ifilename;
  std::string ofilename;
  std::string icolorName;
  std::string ocolorName;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "debug") == 0)
        debug = true;
      else
        std::cerr << "Invalid option " << argv[i];
    }
    else {
      if      (ifilename == "")
        ifilename = argv[i];
      else if (ofilename == "")
        ofilename = argv[i];
      else if (icolorName == "")
        icolorName = argv[i];
      else if (ocolorName == "")
        ocolorName = argv[i];
      else
        std::cerr << "Invalid argument " << argv[i];
    }
  }

  if (ifilename == "" || ofilename == "") {
    std::cerr << "Usage: CImageColorReplace [-debug] "
                 "<ifile> <ofile> <icolor> <ocolor>" << std::endl;
    exit(1);
  }

  CRGBA icolor = CRGBName::toRGBA(icolorName);
  CRGBA ocolor = CRGBName::toRGBA(ocolorName);

  CFile ifile(ifilename);
  CFile ofile(ofilename);

  CImageState::setDebug(debug);

  CImageFileSrc src(ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image.isValid())
    exit(1);

  CImagePtr oimage = image->dup();

  oimage->convertToRGB();

  int w = image->getWidth();
  int h = image->getHeight();

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba;

      image->getRGBAPixel(x, y, rgba);

      if (rgba == icolor)
        oimage->setRGBAPixel(x, y, ocolor);
      else
        oimage->setRGBAPixel(x, y, rgba);
    }
  }

  CFileType type = CFileUtil::getImageTypeFromName(ofilename);

  oimage->write(ofilename, type);

  exit(0);
}
