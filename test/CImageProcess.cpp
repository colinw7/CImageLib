#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <CStrParse.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    std::cerr << "Usage: CImageProcess " <<
                 "<ifile> <ofile> <cmdfile>" << std::endl;
    exit(1);
  }

  CFile ifile(argv[1]);
  CFile ofile(argv[2]);
  CFile cfile(argv[3]);

  CImageFileSrc src(ifile);

  CImagePtr src_image = CImageMgrInst->createImage(src);
  CImagePtr dst_image = src_image->dup();

  std::string line;

  while (cfile.readLine(line)) {
    CStrParse parse(line);

    parse.skipSpace();

    std::string cmd;

    parse.readIdentifier(cmd);

    if      (cmd == "hflip")
      dst_image->flipH();
    else if (cmd == "vflip")
      dst_image->flipV();
    else if (cmd == "resize") {
      parse.skipSpace();

      int w, h;

      if (! parse.readInteger(&w)) { std::cerr << "Bad Resize Width" << std::endl; continue; }

      parse.skipSpace();

      if (! parse.readInteger(&h)) { std::cerr << "Bad Resize Width" << std::endl; continue; }

      dst_image->reshape(w, h);
    }
    else if (cmd == "gray") {
      dst_image->grayScale();
    }
    else
      std::cerr << "Invalid command " << cmd << std::endl;
  }

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  dst_image->write(&ofile, type);

  exit(0);
}
