#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CDir.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "Usage: CImageThumbnails " << "<size> <ifiles>" << std::endl;
    exit(1);
  }

  int size = CStrUtil::toInteger(argv[1]);

  if (size < 0) exit(1);

  if (! CFile::isDirectory(".thumbnails"))
    CDir::makeDir(".thumbnails");

  CImage::setResizeType(CIMAGE_RESIZE_BILINEAR);

  for (int i = 2; i < argc; ++i) {
    if (! CFile::isRegular(argv[i])) continue;

    CFile file(argv[i]);

    CImageFileSrc src(file);

    CImagePtr image = CImageMgrInst->createImage(src);

    if (! image.isValid()) continue;

    if ((int) image->getWidth() > size || (int) image->getHeight() > size)
      image = image->resizeKeepAspect(size, size);

    image->write(".thumbnails/" + std::string(argv[i]));
  }

  exit(0);
}
