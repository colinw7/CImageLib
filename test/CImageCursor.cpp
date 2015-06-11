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
      else
        cerr << "Invalid argument " << argv[i];
    }
  }

  if (ifilename == "" || ofilename == "") {
    cerr << "Usage: CImageCursor [-debug] <ifile> <ofile>" << endl;
    exit(1);
  }

  CFile ifile(ifilename);

  string ofilename1 = ofilename + ".xbm";
  string ofilename2 = ofilename + "mask.xbm";

  CFile ofile1(ofilename1);
  CFile ofile2(ofilename2);

  CImageState::setDebug(debug);

  CImageFileSrc src(ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image.isValid())
    exit(1);

  CImagePtr oimage1 = image->dup();
  CImagePtr oimage2 = image->dup();

  oimage1->convertToRGB();
  oimage2->convertToRGB();

  int w = image->getWidth();
  int h = image->getHeight();

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba;

      image->getRGBAPixel(x, y, rgba);

      int b, m;

      if (rgba.isTransparent()) {
        b = 0;
        m = 0;
      }
      else {
        double gray = rgba.getGray();

        if (gray < 0.5) { // black
          b = 1;
          m = 1;
        }
        else { // white
          b = 0;
          m = 1;
        }
      }

      oimage1->setRGBAPixel(x, y, b ? CRGBA(0,0,0) : CRGBA(1,1,1));
      oimage2->setRGBAPixel(x, y, m ? CRGBA(0,0,0) : CRGBA(1,1,1));
    }
  }

  oimage1->writeXBM(&ofile1);
  oimage2->writeXBM(&ofile2);

  exit(0);
}
