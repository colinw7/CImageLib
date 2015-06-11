#include <std_c++.h>
#include <CImageLib/CImageLib.h>
#include <CFileUtil/CFileUtil.h>

int
main(int argc, char **argv)
{
  for (int i = 1; i < argc; ++i) {
    CFileType type = CFileUtil::getImageType(argv[i]);

    if (type == CFILE_TYPE_NONE)
      continue;

    CImageNameSrc src("CImageLs");

    CImagePtr image = CImageMgrInst->createImage(src);

    image->readHeader(argv[i]);

    if (image.isValid())
      cout << argv[i] << " " << image->getTypeName() << " " <<
              image->getWidth() << "x" << image->getHeight() << endl;
    else
      cout << argv[i] << " ??" << endl;
  }

  return 0;
}
