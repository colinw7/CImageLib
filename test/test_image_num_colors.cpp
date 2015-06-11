#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>
#include <CFileUtil/CFileUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    cerr << "Usage: test_image_num_colors " <<
            "<ifile> <ofile> <num_colors>" << endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);
  int    num   = atoi(argv[3]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  image->setNumColors(num);

  CFileType type = CFileUtil::getImageType(argv[2]);

  image->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
