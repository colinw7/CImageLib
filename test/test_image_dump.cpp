#include <std_c++.h>
#include <CFile/CFile.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc != 2) {
    cerr << "Usage: dump_image <file>" << endl;
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
