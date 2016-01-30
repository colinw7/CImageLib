#include <CImageLib.h>
#include <CImageGIF.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 4) {
    std::cerr << "Usage: CImageGIFAnim " << "<delay> <ofile> <ifiles>" << std::endl;
    exit(1);
  }

  int delay = CStrUtil::toInteger(argv[1]);

  CFile *ofile = new CFile(argv[2]);

  std::vector<CImagePtr> images;

  int num_files = argc - 3;

  for (int i = 0; i < num_files; ++i) {
    CFile ifile(argv[i + 3]);

    CImageFileSrc src(ifile);

    CImagePtr image = CImageMgrInst->createImage(src);

    if (image.isValid())
      images.push_back(image);
  }

  CImageGIF::writeAnim(ofile, images, delay);

  delete ofile;

  exit(0);
}
