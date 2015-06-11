#include <std_c++.h>
#include <CFile/CFile.h>
#include <CStrUtil/CStrUtil.h>
#include <CImageLib/CImageLib.h>

string
indToString(const char *format, int ix, int iy)
{
  char buffer[1024];

  sprintf(buffer, format, ix, iy);

  return buffer;
}

int
main(int argc, char **argv)
{
  bool        flip_dir = false;
  const char *format   = "_%d_%d";

  char *filename = NULL;
  char *dx_str   = NULL;
  char *dy_str   = NULL;
  char *sx_str   = NULL;
  char *sy_str   = NULL;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (strcmp(&argv[i][1], "flip_dir") == 0)
        flip_dir = true;
      else if (strcmp(&argv[i][1], "format") == 0) {
        if (i < argc - 1)
          format = argv[++i];
      }
      else
        cerr << "Invalid option: " << argv[i] << endl;
    }
    else {
      if      (! filename) filename = argv[i];
      else if (! dx_str  ) dx_str   = argv[i];
      else if (! dy_str  ) dy_str   = argv[i];
      else if (! sx_str  ) sx_str   = argv[i];
      else if (! sy_str  ) sy_str   = argv[i];
    }
  }

  if (! filename || ! dx_str || ! dy_str) {
    cerr << "Usage: CImageUntile [-flip_dir] [-format <format>] "
            "<ifile> <dx> <dy> [<sx>] [<sy>]" << endl;
    exit(1);
  }

  CFile *ifile = new CFile(filename);

  int dx = CStrUtil::toInteger(dx_str);
  int dy = CStrUtil::toInteger(dy_str);

  if (dx <= 0 || dy <= 0) return false;

  int sx = (sx_str ? CStrUtil::toInteger(sx_str) : 0);
  int sy = (sy_str ? CStrUtil::toInteger(sy_str) : 0);

  CImageFileSrc src(*ifile);

  CImagePtr image = CImageMgrInst->createImage(src);

  if (! image.isValid())
    exit(1);

  CFileType type = image->getType();

  int width  = image->getWidth ();
  int height = image->getHeight();

  if (! flip_dir) {
    for (int y = 0, iy = 1; y < height; y += dy + sy, ++iy) {
      for (int x = 0, ix = 1; x < width; x += dx + sx, ++ix) {
        CImagePtr image1 = image->subImage(x, y, dx, dy);

        string ind_str = indToString(format, ix, iy);

        string filename = ifile->getDir() + "/" + ifile->getBase() +
                          ind_str + "." + ifile->getSuffix();

        image1->write(filename, type);
      }
    }
  }
  else {
    for (int x = 0, ix = 1; x < width; x += dx + sx, ++ix) {
      for (int y = 0, iy = 1; y < height; y += dy + sy, ++iy) {
        CImagePtr image1 = image->subImage(x, y, dx, dy);

        string ind_str = indToString(format, iy, ix);

        string filename = ifile->getDir() + "/" + ifile->getBase() +
                          ind_str + "." + ifile->getSuffix();

        image1->write(filename, type);
      }
    }
  }

  delete ifile;

  exit(0);
}
