#include <CImageLib.h>
#include <CFile.h>
#include <CFileUtil.h>
#include <CStrUtil.h>
#include <cstring>

#define UNSET_INT -999

int
main(int argc, char **argv)
{
  std::string title = "Browse Images";
  std::string h1    = "";
  std::string bg    = "";

  int x_size = UNSET_INT;
  int y_size = UNSET_INT;
  int width  = UNSET_INT;

  std::string              ostr;
  std::vector<std::string> istrs;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (strcmp(&argv[i][1], "title") == 0) {
        ++i;

        if (i < argc)
          title = argv[i];
      }
      else if (strcmp(&argv[i][1], "h1") == 0) {
        ++i;

        if (i < argc)
          h1 = argv[i];
      }
      else if (strcmp(&argv[i][1], "bg") == 0) {
        ++i;

        if (i < argc)
          bg = argv[i];
      }
      else {
        std::cerr << "Invalid arg: " << argv[i] << std::endl;
        exit(1);
      }
    }
    else {
      if      (x_size == UNSET_INT) x_size = CStrUtil::toInteger(argv[i]);
      else if (y_size == UNSET_INT) y_size = CStrUtil::toInteger(argv[i]);
      else if (width  == UNSET_INT) width  = CStrUtil::toInteger(argv[i]);
      else if (ostr   == ""       ) ostr   = argv[i];
      else                          istrs.push_back(argv[i]);
    }
  }

  if (x_size == UNSET_INT || y_size == UNSET_INT || width == UNSET_INT ||
      ostr == "" || istrs.empty()) {
    std::cerr << "Usage: CImageHTMLGen " <<
                 "<x_size> <y_size> <width> <ofile> <ifiles>" << std::endl;
    exit(1);
  }

  CFile *ofile = new CFile(ostr);

  int num_x = width/x_size;

  if ((width % x_size) != 0)
    num_x++;

  int num_files = istrs.size();

  int num_y = num_files/num_x;

  if ((num_files % num_x) != 0)
    num_y++;

  if (num_y == 1 && num_x > num_files)
    num_x = num_files;

  std::cout << "<html>" << std::endl;
  std::cout << "<head>" << std::endl;
  std::cout << "<title>" << title << "</title>" << std::endl;

  if (bg != "")
    std::cout << "<body bgcolor=" << bg << ">" << std::endl;
  else
    std::cout << "<body>" << std::endl;

  if (h1 != "")
    std::cout << "<h1>" << h1 << "</h1>" << std::endl;

  std::cout << "<img src=\"" << ostr << "\" usemap=\"#map1\">" << std::endl;
  std::cout << "<map name=\"map1\">" << std::endl;

  CImage::setResizeType(CIMAGE_RESIZE_BILINEAR);

  CImageNameSrc src("CImageHTMLGen");

  CImagePtr image = CImageMgrInst->createImage(src);

  image->setDataSize(num_x*x_size, num_y*y_size);

  int x = 0;
  int y = 0;

  int k = 0;

  for (int i = 0; i < num_y; i++, y += y_size) {
    x = 0;

    for (int j = 0; j < num_x; j++, k++, x += x_size) {
      if (k >= num_files)
        break;

      CFile ifile(istrs[k]);

      CImageFileSrc src(ifile);

      CImagePtr image1 = CImageMgrInst->createImage(src);

      if (image1.isValid()) {
        CImagePtr image2;

        if (x_size < int(image1->getWidth()) || y_size < int(image1->getHeight())) {
          int dx = abs(image1->getWidth () - x_size);
          int dy = abs(image1->getHeight() - y_size);

          if      (dx > dy)
            image2 = image1->resizeWidth (x_size);
          else if (dy > dx)
            image2 = image1->resizeHeight(y_size);
          else if (dx > 0)
            image2 = image1->resizeWidth (x_size);
          else
            image2 = image1;
        }
        else
          image2 = image1;

        image2->convertToRGB();

        std::cout << "<area shape=rect coords=\"" <<
                     x << "," << y << "," << (x + x_size) << "," << (y + y_size) <<
                     "\" href=\"" << istrs[k] << "\">" << std::endl;

        int w = std::min((int) image2->getWidth (), x_size);
        int h = std::min((int) image2->getHeight(), y_size);

        image->subCopyFrom(image2, 0, 0, x_size, y_size,
                           x + (x_size - w)/2, y + (y_size - h)/2);
      }
    }
  }

  std::cout << "</map>" << std::endl;
  std::cout << "</body>" << std::endl;
  std::cout << "</html>" << std::endl;

  CFileType type = CFileUtil::getImageTypeFromName(ostr);

  image->write(ofile, type);

  delete ofile;

  exit(0);
}
