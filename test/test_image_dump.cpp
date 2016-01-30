#include <CImageLib.h>
#include <CFile.h>

int
main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: dump_image <file>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);

  CFile *ofile = new CFile("dump.xpm");

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  image->writeXPM(ofile);

  delete ifile;
  delete ofile;

  exit(0);
}
