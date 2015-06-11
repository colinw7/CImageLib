#include <std_c++.h>
#include <CFile/CFile.h>
#include <CFileUtil/CFileUtil.h>
#include <CStrUtil/CStrUtil.h>
#include <CStrParse/CStrParse.h>
#include <CImageLib/CImageLib.h>

int
main(int argc, char **argv)
{
  if (argc != 4) {
    cerr << "Usage: CImageProcess " <<
            "<ifile> <ofile> <cmdfile>" << endl;
    exit(1);
  }

  CFile ifile(argv[1]);
  CFile ofile(argv[2]);
  CFile cfile(argv[3]);

  CImageFileSrc src(ifile);

  CImagePtr src_image = CImageMgrInst->createImage(src);
  CImagePtr dst_image = src_image->dup();

  string line;

  while (cfile.readLine(line)) {
    CStrParse parse(line);

    parse.skipSpace();

    string cmd;

    parse.readIdentifier(cmd);

    if      (cmd == "hflip")
      dst_image->flipH();
    else if (cmd == "vflip")
      dst_image->flipV();
    else if (cmd == "resize") {
      parse.skipSpace();

      int w, h;

      if (! parse.readInteger(&w)) { cerr << "Bad Resize Width" << endl; continue; }

      parse.skipSpace();

      if (! parse.readInteger(&h)) { cerr << "Bad Resize Width" << endl; continue; }

      dst_image->reshape(w, h);
    }
    else if (cmd == "gray") {
      dst_image->grayScale();
    }
    else
      cerr << "Invalid command " << cmd << endl;
  }

  CFileType type = CFileUtil::getImageTypeFromName(argv[2]);

  dst_image->write(&ofile, type);

  exit(0);
}
