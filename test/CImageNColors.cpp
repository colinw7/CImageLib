#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <cstring>

int
main(int argc, char **argv)
{
  int  n     = 256;
  bool debug = false;

  CImage::ConvertMethod method = CImage::CONVERT_NEAREST_LOGICAL;

  std::string ifilename;
  std::string ofilename;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (strcmp(&argv[i][1], "debug") == 0)
        debug = true;
      else if (strcmp(&argv[i][1], "n") == 0) {
        ++i;

        if (i >= argc) {
          std::cerr << "Missing Value for -n" << std::endl;
          exit(1);
        }

        n = atoi(argv[i]);

        if (n < 2 || n > 256) {
          std::cerr << "Illegal Value for -n" << std::endl;
          exit(1);
        }
      }
      else if (strcmp(&argv[i][1], "logical") == 0)
        method = CImage::CONVERT_NEAREST_LOGICAL;
      else if (strcmp(&argv[i][1], "physical") == 0)
        method = CImage::CONVERT_NEAREST_PHYSICAL;
      else
        std::cerr << "Invalid option " << argv[i];
    }
    else {
      if      (ifilename == "")
        ifilename = argv[i];
      else if (ofilename == "")
        ofilename = argv[i];
      else
        std::cerr << "Invalid argument " << argv[i];
    }
  }

  if (ifilename == "" || ofilename == "") {
    std::cerr << "Usage: CImageNColors [-n <n>] [-logical|-physical] [-debug] "
                 "<ifile> <ofile>" << std::endl;
    exit(1);
  }

  CFile ifile(ifilename);
  CFile ofile(ofilename);

  CImageState::setDebug(debug);

  CImageFileSrc src(ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  image->convertToNColors(n, method);

  CFileType type = CFileUtil::getImageTypeFromName(ofilename);

  image->write(&ofile, type);

  exit(0);
}
