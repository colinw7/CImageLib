#include <CImageLib.h>
#include <CFile.h>
#include <CDir.h>
#include <CStrUtil.h>

int
main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "Usage: test_image_html_frame " <<
                 "<size> <name> <ifiles>" << std::endl;
    exit(1);
  }

  int size = CStrUtil::toInteger(argv[1]);

  std::string hname = argv[2];

  int num_files = argc - 3;

  std::vector<std::string> names, bases;

  for (int i = 0; i < num_files; ++i) {
    std::string name = argv[i + 3];

    names.push_back(name);

    CFile file(name);

    bases.push_back(file.getBase());
  }

  //-----

  CDir::makeDir("thumbnails");

  //-----

  // Create thumbnails

  for (int i = 0; i < num_files; ++i) {
    CFile ifile(names[i]);

    CImageFileSrc src(ifile);

    CImagePtr image = CImageMgrInst->createImage(src);

    if (! image)
      continue;

    CImagePtr image1 = image->resizeMax(size);

    CFile ofile1("thumbnails/" + bases[i] + ".jpg");

    image1->writeJPG(&ofile1);

    CFile ofile2("thumbnails/" + bases[i] + "_sepia.jpg");

    image1->sepia();

    image1->writeJPG(&ofile2);
  }

  //-----

  // Write master page

  CFile hfile1(hname + ".html");

  hfile1.printf("<html>\n");
  hfile1.printf("<head>\n");
  hfile1.printf("<title>Browse Images</title>\n");
  hfile1.printf("</head>\n");
  hfile1.printf("<frameset cols=\"150,*\" framespacing=0 border=0 frameborder=no>\n");
  hfile1.printf("<frame name=\"thumb\" src=\"%s_thumb.html\" marginheight=9 marginwidth=9 noresize>\n", hname.c_str());
  hfile1.printf("<frame name=\"image\" src=\"%s_image.html\" scrolling=auto noresize>\n");
  hfile1.printf("</frameset>\n");
  hfile1.printf("<body>\n");
  hfile1.printf("</body>\n");
  hfile1.printf("</html>\n");

  //-----

  // Write Thumbnail page

  CFile hfile2(hname + "_thumb.html");

  hfile2.printf("<html>\n");
  hfile2.printf("<head>\n");
  hfile2.printf("<title>Browse Images (Thumbnails)</title>\n");
  hfile2.printf("<body>\n");

  hfile2.printf("<script language=\"JavaScript1.1\">\n");

  for (int i = 0; i < num_files; ++i) {
    const char *base = bases[i].c_str();

    hfile2.printf("function %s_enter() {\n", base);
    hfile2.printf("  document.images[\"%s\"].src=\"thumbnails/%s.jpg\"\n", base, base);
    hfile2.printf("}\n");

    hfile2.printf("function %s_leave() {\n", base);
    hfile2.printf("  document.images[\"%s\"].src=\"thumbnails/%s_sepia.jpg\"\n", base, base);
    hfile2.printf("}\n");
  }

  hfile2.printf("</script>\n");

  hfile2.printf("<body>\n");

  for (int i = 0; i < num_files; ++i) {
    const char *base = bases[i].c_str();
    const char *name = names[i].c_str();

    hfile2.printf("<p><a href=\"%s\" target=\"image\" onMouseOver=\"%s_enter();\" onMouseOut=\"%s_leave();\">\n", name, base, base);
    hfile2.printf("<img src=\"thumbnails/%s_sepia.jpg\" name=\"%s\" border=0></a></li>\n", base, base);
  }

  hfile2.printf("</body>\n");
  hfile2.printf("</html>\n");

  //-----

  // Write Image page

  CFile hfile3(hname + "_image.html");

  hfile3.printf("<html>\n");
  hfile3.printf("<head>\n");
  hfile3.printf("<title>Browse Images (Image)</title>\n");
  hfile3.printf("<body>\n");
  hfile3.printf("</body>\n");
  hfile3.printf("</html>\n");

  exit(0);
}
