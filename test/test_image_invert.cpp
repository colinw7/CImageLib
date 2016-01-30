#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << "Usage: test_image_invert " <<
                 "<ifile> <ofile>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  CImageFileSrc src1(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src1);

  if (! image)
    exit(1);

  image->invert();

  CFileType type = CFileUtil::getImageType(argv[2]);

  image->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
