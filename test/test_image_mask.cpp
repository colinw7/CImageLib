#include <CImageLib.h>
#include <CFile.h>

static void testCreateMask();
static void testMask1();
static void testMask2();

int
main(int /*argc*/, char ** /*argv*/)
{
  testCreateMask();

  testMask1();
  testMask2();

  exit(0);
}

static void
testCreateMask()
{
  CFile *ifile = new CFile("data/transparent.xpm");

  CFile *ofile = new CFile("test_mask.xpm");

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image)
    exit(1);

  CImagePtr image1 = image->createMask();

  if (! image1)
    exit(1);

  image1->writeXPM(ofile);

  delete ifile;
  delete ofile;
}

static void
testMask1()
{
  CFile *ifile1 = new CFile("data/image.iff");
  CFile *ifile2 = new CFile("data/mask_iff.gif");

  CFile *ofile = new CFile("image_iff.gif");

  CImageFileSrc src1(*ifile1);

  CImagePtr image1 = CImageMgrInst->createImage(src1);

  CImageFileSrc src2(*ifile2);

  CImagePtr image2 = CImageMgrInst->createImage(src2);

  image2->setAlphaByGray(false);

  image1->alphaMask(image2);

  image1->writeGIF(ofile);

  delete ifile1;
  delete ifile2;
  delete ofile;
}

static void
testMask2()
{
  CFile *ifile1 = new CFile("data/transparent.png");
  CFile *ifile2 = new CFile("data/png_mask.png");

  CFile *ofile = new CFile("transparent1.png");

  CImageFileSrc src1(*ifile1);

  CImagePtr image1 = CImageMgrInst->createImage(src1);

  CImageFileSrc src2(*ifile2);

  CImagePtr image2 = CImageMgrInst->createImage(src2);

  image1->alphaMask(image2);

  image1->writePNG(ofile);

  delete ifile1;
  delete ifile2;
  delete ofile;
}
