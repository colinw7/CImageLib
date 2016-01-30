#include <CImageLib.h>
#include <CFile.h>
#include <COSTime.h>

int
main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: test_image_iter <ifile>" << std::endl;
    exit(1);
  }

  CFile ifile(argv[1]);

  CImageFileSrc src(ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  CRGB rgb;

  double secs;
  int    secs1, usecs1, secs2, usecs2;

  {
  COSTime::getHRTime(&secs1, &usecs1);

  CImage::pixel_iterator p1 = image->pixel_begin();
  CImage::pixel_iterator p2 = image->pixel_end  ();

  for ( ; p1 != p2; ++p1)
    p1.getRGBPixel(rgb);

  COSTime::getHRTime(&secs2, &usecs2);

  COSTime::diffHRTime(secs1, usecs1, secs2, usecs2, &secs);

  std::cout << secs << std::endl;
  }

  {
  COSTime::getHRTime(&secs1, &usecs1);

  uint width  = image->getWidth ();
  uint height = image->getHeight();

  uint size = width*height;

  for (uint pos = 0; pos < size; ++pos)
    image->getRGBPixel(pos, rgb);

  COSTime::getHRTime(&secs2, &usecs2);

  COSTime::diffHRTime(secs1, usecs1, secs2, usecs2, &secs);

  std::cout << secs << std::endl;
  }

  exit(0);
}
