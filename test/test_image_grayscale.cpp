#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>
#include <CFileUtil/CFileUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 3) {
    cerr << "Usage: test_image_grayscale " <<
            "<ifile> <ofile>" << endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  image->grayScale();

  CFileType type = CFileUtil::getImageType(argv[2]);

  image->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
