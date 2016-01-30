#include <CImageLib.h>
#include <CFile.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc != 7) {
    std::cerr << "Usage: test_image_tile <ifile> <ofile> <width> " <<
                 "<height> <halign> <valign>" << std::endl;
    exit(1);
  }

  CFile *ifile = new CFile(argv[1]);
  CFile *ofile = new CFile(argv[2]);

  int width  = CStrUtil::toInteger(argv[3]);
  int height = CStrUtil::toInteger(argv[4]);

  CHAlignType halign = CHALIGN_TYPE_LEFT;

  if      (argv[5][0] == 'l')
    halign = CHALIGN_TYPE_LEFT;
  else if (argv[5][0] == 'c')
    halign = CHALIGN_TYPE_CENTER;
  else if (argv[5][0] == 'r')
    halign = CHALIGN_TYPE_RIGHT;

  CVAlignType valign = CVALIGN_TYPE_TOP;

  if      (argv[6][0] == 't')
    valign = CVALIGN_TYPE_TOP;
  else if (argv[6][0] == 'c')
    valign = CVALIGN_TYPE_CENTER;
  else if (argv[6][0] == 'b')
    valign = CVALIGN_TYPE_BOTTOM;

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  CImageTile tile(halign, valign);

  CImagePtr image1 = image->tile(width, height, tile);

  image1->writeBMP(ofile);

  delete ifile;
  delete ofile;

  exit(0);
}
