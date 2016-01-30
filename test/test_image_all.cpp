#include <CImageLib.h>
#include <CFile.h>

int
main(int /*argc*/, char ** /*argv*/)
{
  CFile *ifile1 = new CFile("data/image.gif");
  CFile *ifile2 = new CFile("data/image.png");
  CFile *ifile3 = new CFile("data/image.iff");
  CFile *ifile4 = new CFile("data/image.pcx");
  CFile *ifile5 = new CFile("data/image.rgb");
  CFile *ifile6 = new CFile("data/image.psp");

  CFile *ofile1 = new CFile("test.xpm");
  CFile *ofile2 = new CFile("test.bmp");
  CFile *ofile3 = new CFile("test.jpg");
  CFile *ofile4 = new CFile("test.tif");
  CFile *ofile5 = new CFile("test.xbm");
  CFile *ofile6 = new CFile("test.xwd");
  CFile *ofile7 = new CFile("test.ps");
  CFile *ofile8 = new CFile("test.png");

  CFile *tfile1 = new CFile("test_xpm.gif");
  CFile *tfile2 = new CFile("test_bmp.gif");
  CFile *tfile3 = new CFile("test_jpg.bmp");
  CFile *tfile4 = new CFile("test_tif.bmp");
  CFile *tfile5 = new CFile("test_xbm.bmp");
  CFile *tfile6 = new CFile("test_xwd.bmp");

  CFile *wfile1 = new CFile("test_png.bmp");
  CFile *wfile2 = new CFile("test_iff.bmp");
  CFile *wfile3 = new CFile("test_pcx.bmp");
  CFile *wfile4 = new CFile("test_sgi.bmp");
  CFile *wfile5 = new CFile("test_psp.bmp");

  CFile *gfile1 = new CFile("test_gen.bmp");

  //------

  CImageFileSrc src(*ifile1);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (image.isValid()) {
    image->writeXPM(ofile1);
    image->writeBMP(ofile2);
    image->writeJPG(ofile3);
    image->writeTIF(ofile4);
    image->writeXBM(ofile5);
    image->writeXWD(ofile6);
    image->writePS (ofile7);
    image->writePNG(ofile8);
  }

  //------

  CImageFileSrc src1(*ofile1);

  image = CImageMgrInst->createImage(src1);

  if (image.isValid())
    image->writeGIF(tfile1);

  CImageFileSrc src2(*ofile2);

  image = CImageMgrInst->createImage(src2);

  if (image.isValid())
    image->writeGIF(tfile2);

  CImageFileSrc src3(*ofile3);

  image = CImageMgrInst->createImage(src3);

  if (image.isValid())
    image->writeBMP(tfile3);

  CImageFileSrc src4(*ofile4);

  image = CImageMgrInst->createImage(src4);

  if (image.isValid())
    image->writeBMP(tfile4);

  CImageFileSrc src5(*ofile5);

  image = CImageMgrInst->createImage(src5);

  if (image.isValid())
    image->writeBMP(tfile5);

  CImageFileSrc src6(*ofile6);

  image = CImageMgrInst->createImage(src6);

  if (image.isValid())
    image->writeBMP(tfile6);

  //------

  CImageFileSrc src7(*ifile2);

  image = CImageMgrInst->createImage(src7);

  if (image.isValid())
    image->writeBMP(wfile1);

  CImageFileSrc src8(*ifile3);

  image = CImageMgrInst->createImage(src8);

  if (image.isValid())
    image->writeBMP(wfile2);

  CImageFileSrc src9(*ifile4);

  image = CImageMgrInst->createImage(src9);

  if (image.isValid())
    image->writeBMP(wfile3);

  CImageFileSrc src10(*ifile5);

  image = CImageMgrInst->createImage(src10);

  if (image.isValid())
    image->writeBMP(wfile4);

  CImageFileSrc src11(*ifile6);

  image = CImageMgrInst->createImage(src11);

  if (image.isValid())
    image->writeBMP(wfile5);

  //------

  ifile1->rewind();

  CImageFileSrc src12(*ifile1);

  image = CImageMgrInst->createImage(src12);

  if (image.isValid())
    image->writeBMP(gfile1);

  //------

  delete ifile1;
  delete ifile2;
  delete ifile3;
  delete ifile4;
  delete ifile5;
  delete ifile6;

  delete ofile1;
  delete ofile2;
  delete ofile3;
  delete ofile4;
  delete ofile5;
  delete ofile6;
  delete ofile7;

  delete tfile1;
  delete tfile2;
  delete tfile3;
  delete tfile4;
  delete tfile5;
  delete tfile6;

  delete wfile1;
  delete wfile2;
  delete wfile3;
  delete wfile4;
  delete wfile5;

  delete gfile1;

  exit(0);
}
