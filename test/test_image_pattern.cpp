#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 4) {
    std::cerr << "Usage: test_image_pattern " <<
                 "<ofile> <pattern> <w> <h>" << std::endl;
    exit(1);
  }

  CFile ofile(argv[1]);
  CFile pfile(argv[2]);

  int w, h;

  if (! CStrUtil::toInteger(argv[3], &w) ||
      ! CStrUtil::toInteger(argv[4], &h))
    exit(1);

  CImagePtr pimage = CImageMgrInst->createImage();

  pimage->read(&pfile);

  CImagePtr oimage = CImageMgrInst->createImage();

  oimage->setDataSize(w, h);

  oimage->patternFill(pimage);

  CFileType type = CFileUtil::getImageTypeFromName(argv[1]);

  oimage->write(&ofile, type);

  exit(0);
}
