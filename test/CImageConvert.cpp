#include <CImageLib.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <CFileUtil.h>
#include <CRGBName.h>
#include <cstring>

int
main(int argc, char **argv)
{
  bool debug = false;

  std::string ifilename;
  std::string ofilename;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (strcmp(&argv[i][1], "debug") == 0)
        debug = true;
      else if (strcmp(&argv[i][1], "bg") == 0) {
        ++i;

        if (i < argc)
          CImage::setConvertBg(CRGBName::toRGBA(argv[i]));
        else
          std::cerr << "Missing value for -bg";
      }
      else
        std::cerr << "Invalid option " << argv[i] << std::endl;
    }
    else {
      if      (ifilename == "")
        ifilename = argv[i];
      else if (ofilename == "")
        ofilename = argv[i];
      else
        std::cerr << "Invalid argument " << argv[i] << std::endl;
    }
  }

  if (ifilename == "" || ofilename == "") {
    std::cerr << "Usage: CImageConvert [-debug] <ifile> <ofile>" << std::endl;
    exit(1);
  }

  CFile ifile(ifilename);
  CFile ofile(ofilename);

  CImageState::setDebug(debug);

  CImageFileSrc src(ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image.isValid())
    exit(1);

  CFileType type = CFileUtil::getImageTypeFromName(ofilename);

  image->write(&ofile, type);

  exit(0);
}
