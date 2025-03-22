#include <CImageLib.h>
#include <CFile.h>

int
main(int /*argc*/, char ** /*argv*/)
{
  auto *ifile1 = new CFile("data/image.gif");
  auto *ifile2 = new CFile("data/image.png");
  auto *ifile3 = new CFile("data/image.iff");
  auto *ifile4 = new CFile("data/image.pcx");
  auto *ifile5 = new CFile("data/image.rgb");
  auto *ifile6 = new CFile("data/image.psp");

  auto *ofile1 = new CFile("test.xpm");
  auto *ofile2 = new CFile("test.bmp");
  auto *ofile3 = new CFile("test.jpg");
  auto *ofile4 = new CFile("test.tif");
  auto *ofile5 = new CFile("test.xbm");
  auto *ofile6 = new CFile("test.xwd");
  auto *ofile7 = new CFile("test.ps");
  auto *ofile8 = new CFile("test.png");

  auto *tfile1 = new CFile("test_xpm.gif");
  auto *tfile2 = new CFile("test_bmp.gif");
  auto *tfile3 = new CFile("test_jpg.bmp");
  auto *tfile4 = new CFile("test_tif.bmp");
  auto *tfile5 = new CFile("test_xbm.bmp");
  auto *tfile6 = new CFile("test_xwd.bmp");

  auto *wfile1 = new CFile("test_png.bmp");
  auto *wfile2 = new CFile("test_iff.bmp");
  auto *wfile3 = new CFile("test_pcx.bmp");
  auto *wfile4 = new CFile("test_sgi.bmp");
  auto *wfile5 = new CFile("test_psp.bmp");

  auto *gfile1 = new CFile("test_gen.bmp");

  //------

  CImageFileSrc src(*ifile1);

  auto image = CImageMgrInst->createImage(src);

  if (image) {
    image->writeXPM(ofile1); ofile1->flush();
    image->writeBMP(ofile2); ofile2->flush();
    image->writeJPG(ofile3); ofile3->flush();
    image->writeTIF(ofile4); ofile4->flush();
    image->writeXBM(ofile5); ofile5->flush();
    image->writeXWD(ofile6); ofile6->flush();
    image->writePS (ofile7); ofile7->flush();
    image->writePNG(ofile8); ofile8->flush();
  }

  //------

  CImageFileSrc src1(*ofile1);

  image = CImageMgrInst->createImage(src1);

  if (image)
    image->writeGIF(tfile1);

  CImageFileSrc src2(*ofile2);

  image = CImageMgrInst->createImage(src2);

  if (image)
    image->writeGIF(tfile2);

  CImageFileSrc src3(*ofile3);

  image = CImageMgrInst->createImage(src3);

  if (image)
    image->writeBMP(tfile3);

  CImageFileSrc src4(*ofile4);

  image = CImageMgrInst->createImage(src4);

  if (image)
    image->writeBMP(tfile4);

  CImageFileSrc src5(*ofile5);

  image = CImageMgrInst->createImage(src5);

  if (image)
    image->writeBMP(tfile5);

  CImageFileSrc src6(*ofile6);

  image = CImageMgrInst->createImage(src6);

  if (image)
    image->writeBMP(tfile6);

  //------

  CImageFileSrc src7(*ifile2);

  image = CImageMgrInst->createImage(src7);

  if (image)
    image->writeBMP(wfile1);

  CImageFileSrc src8(*ifile3);

  image = CImageMgrInst->createImage(src8);

  if (image)
    image->writeBMP(wfile2);

  CImageFileSrc src9(*ifile4);

  image = CImageMgrInst->createImage(src9);

  if (image)
    image->writeBMP(wfile3);

  CImageFileSrc src10(*ifile5);

  image = CImageMgrInst->createImage(src10);

  if (image)
    image->writeBMP(wfile4);

  CImageFileSrc src11(*ifile6);

  image = CImageMgrInst->createImage(src11);

  if (image)
    image->writeBMP(wfile5);

  //------

  ifile1->rewind();

  CImageFileSrc src12(*ifile1);

  image = CImageMgrInst->createImage(src12);

  if (image)
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
