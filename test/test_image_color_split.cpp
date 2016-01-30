#include <CImageLib.h>
#include <CFile.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: test_image_color_split <ifile>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  CFileType type = CFILE_TYPE_IMAGE_GIF;

  int num_colors = image->getNumColors();

  for (int i = 0; i < num_colors; i++) {
    CImagePtr image1 = image->colorSplit(i);

    image1->removeSinglePixels();

    std::string filename = ifile->getBase() + CStrUtil::toString(i + 1) + ".gif";

    CFile ofile(filename);

    image1->write(&ofile, type);
  }

  delete ifile;

  exit(0);
}
