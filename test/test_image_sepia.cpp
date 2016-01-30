#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << "Usage: test_image_sepia <ifile> <ofile>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  image->sepia();

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  image->write(ofile, type);

  delete ifile;
  delete ofile;

  exit(0);
}
