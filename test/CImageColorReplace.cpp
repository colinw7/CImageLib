#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>
#include <CFileUtil/CFileUtil.h>

int
main(int argc, char **argv)
{
  bool debug = false;

  string ifilename;
  string ofilename;
  string icolorName;
  string ocolorName;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "debug") == 0)
        debug = true;
      else
        cerr << "Invalid option " << argv[i];
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
        cerr << "Invalid argument " << argv[i];
    }
  }

  if (ifilename == "" || ofilename == "") {
    cerr << "Usage: CImageColorReplace [-debug] <ifile> <ofile> <icolor> <ocolor>" << endl;
    exit(1);
  }

  CRGBA icolor = CRGBA(icolorName);
  CRGBA ocolor = CRGBA(ocolorName);

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
