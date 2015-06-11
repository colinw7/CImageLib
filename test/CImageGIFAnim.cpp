#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>
#include <CImageLib/CImageGIF.h>
#include <CFileUtil/CFileUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 4) {
    cerr << "Usage: CImageGIFAnim " <<
            "<delay> <ofile> <ifiles>" << endl;
    exit(1);
  }

  int delay = CStrUtil::toInteger(argv[1]);

  CFile *ofile = new CFile(argv[2]);

  vector<CImagePtr> images;

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
