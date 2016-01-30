#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <cstring>

int
main(int argc, char **argv)
{
  bool debug = false;

  std::string ifilename1, ifilename2;
  std::string ofilename;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "debug") == 0)
        debug = true;
      else
        std::cerr << "Invalid option " << argv[i];
    }
    else {
      if      (ifilename1 == "")
        ifilename1 = argv[i];
      else if (ifilename2 == "")
        ifilename2 = argv[i];
      else if (ofilename == "")
        ofilename  = argv[i];
      else
        std::cerr << "Invalid argument " << argv[i];
    }
  }

  if (ifilename1 == "" || ifilename2 == "" || ofilename == "") {
    std::cerr << "Usage: CImageCursorCombine [-debug] <ifile1> <ifile2> <ofile>" << std::endl;
    exit(1);
  }

  CFile ifile1(ifilename1);
  CFile ifile2(ifilename2);

  CFile ofile(ofilename);

  CImageState::setDebug(debug);

  CImageFileSrc src1(ifile1);
  CImageFileSrc src2(ifile2);

  CImagePtr image1 = CImageMgrInst->createImage(src1);
  CImagePtr image2 = CImageMgrInst->createImage(src2);

  if (! image1.isValid() || ! image2.isValid())
    exit(1);

  CImagePtr oimage = image1->dup();

  oimage->convertToRGB();

  int w = std::min(image1->getWidth (), image2->getWidth ());
  int h = std::min(image1->getHeight(), image2->getHeight());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      CRGBA rgba1, rgba2;

      image1->getRGBAPixel(x, y, rgba1); // normal
      image2->getRGBAPixel(x, y, rgba2); // mask

      int g1 = (rgba1.getGray() < 0.5 ? 1 : 0);
      int g2 = (rgba2.getGray() < 0.5 ? 1 : 0);

      CRGBA rgba;

      if      (g1 == 1 && g2 == 1)
        rgba = CRGBA(0,0,0,1);
      else if (g1 == 0 && g2 == 1)
        rgba = CRGBA(1,1,1,1);
      else if (g1 == 0 && g2 == 0)
        rgba = CRGBA(0,0,0,0);
      else
        rgba = CRGBA(0,0,0,0);

      oimage->setRGBAPixel(x, y, rgba);
    }
  }

  oimage->writeXPM(&ofile);

  exit(0);
}
