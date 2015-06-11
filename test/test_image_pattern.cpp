#include <std_c++.h>
#include <CFile/CFile.h>
#include <CFileUtil/CFileUtil.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc < 4) {
    cerr << "Usage: test_image_pattern " <<
            "<ofile> <pattern> <w> <h>" << endl;
    exit(1);
  }

  CFile ofile(argv[1]);
  CFile pfile(argv[2]);

  int w, h;

  if (! CStrUtil::toInteger(argv[3], &w) ||
      ! CStrUtil::toInteger(argv[4], &h))
    exit(1);

  CImagePtr pimage = CImageMgrInst->createImage();

  pimage->read(pfile);

  CImagePtr oimage = CImageMgrInst->createImage();

  oimage->setDataSize(w, h);

  oimage->patternFill(pimage);

  CFileType type = CFileUtil::getImageTypeFromName(argv[1]);

  oimage->write(&ofile, type);

  exit(0);
}
