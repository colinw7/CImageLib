#include <CImageLib.h>
#include <CImageXPM.h>
#include <CImageColorDef.h>
#include <CThrow.h>
#include <CStrUtil.h>

#include <cstring>

static const std::string xpm_pixel_chars_ =
 ".#abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static int xpm_chars_per_pixel_;
static int xpm_x_hot_;
static int xpm_y_hot_;

struct CImageXPMColor {
  std::string name;
  std::string mono;
  std::string symb;
  std::string grey4;
  std::string grey;
  std::string color;

  CImageXPMColor() :
   name (""),
   mono (""),
   symb (""),
   grey4(""),
   grey (""),
   color("") {
  }

  std::string colorName() const {
    std::string str = color;

    if (str == "")
      str = grey;

    if (str == "")
      str = grey4;

    return str;
  }
};

struct CImageXPMData {
  int             width;
  int             height;
  int             num_colors;
  CImageXPMColor *colors;
  bool            transparent;
  uint            transparent_color;
  int             chars_per_pixel;
  int             x_hot;
  int             y_hot;
  bool            extension;
  uint           *data;

  CImageXPMData() :
   width            (0),
   height           (0),
   num_colors       (0),
   colors           (0),
   transparent      (false),
   transparent_color(0),
   chars_per_pixel  (0),
   x_hot            (0),
   y_hot            (0),
   extension        (false),
   data             (0) {
  }
};

//----------------------

void
CImageXPM::
setHotSpot(int x, int y)
{
  xpm_x_hot_ = x;
  xpm_y_hot_ = y;
}

//----------------------

bool
CImageXPM::
read(CFile *file, CImagePtr &image)
{
  CImageXPMData xpm_data;

  CFileData *file_data = 0;

  try {
    file->rewind();

    file_data = file->readAll();

    if (! file_data) {
      CImage::errorMsg("Failed to read file");
      return false;
    }

    char *data = (char *) file_data->getData();

    //------

    int i = 0;

    //------

    if (! readHeader(data, &i)) {
      CImage::errorMsg("Failed to read header");
      return false;
    }

    //------

    if (! skipDcl(data, &i)) {
      CImage::errorMsg("Failed to read declarations");
      return false;
    }

    //------

    if (! readValues(data, &i, &xpm_data)) {
      CImage::errorMsg("Failed to read values");
      return false;
    }

    //------

    if (! readColors(data, &i, &xpm_data)) {
      CImage::errorMsg("Failed to read colors");
      return false;
    }

    //------

    if (! readData(data, &i, &xpm_data))
      return false;

    //------

    CRGBA *colors = createImageColors(&xpm_data);

    //------

    image->setType(CFILE_TYPE_IMAGE_XPM);

    image->setDataSize(xpm_data.width, xpm_data.height);

    if (xpm_data.num_colors <= 256) {
      for (int i = 0; i < xpm_data.num_colors; ++i)
        image->addColor(colors[i]);

      image->setColorIndexData(xpm_data.data);

      if (xpm_data.transparent)
        image->setTransparentColor(xpm_data.transparent_color);
    }
    else
      image->setRGBAData(xpm_data.data);

    delete [] xpm_data.data;

    //------

    delete [] colors;
    delete [] xpm_data.colors;

    delete file_data;

    //------

    return true;
  }
  catch (...) {
    CImage::errorMsg("Failed to read file");

    delete [] xpm_data.colors;

    delete file_data;

    return false;
  }
}

bool
CImageXPM::
read(const char **strings, uint num_strings, CImagePtr &image)
{
  if (num_strings <= 0) {
    CImage::errorMsg("No data strings");
    return false;
  }

  //------

  CImageXPMData xpm_data;

  try {
    int i   = 0;
    int pos = 0;

    //------

    if (! readValuesString(&xpm_data, strings[i])) {
      CImage::errorMsg("Failed to read values");
      return false;
    }

    ++i;

    //------

    int j;

    xpm_data.colors = new CImageXPMColor [xpm_data.num_colors];

    for (j = 0; j < xpm_data.num_colors; ++j) {
      if (i >= (int) num_strings) {
        CImage::errorMsg("Failed to read colors");
        return false;
      }

      if (! readColorString(&xpm_data, strings[i], &xpm_data.colors[j])) {
        CImage::errorMsg("Failed to read colors");
        return false;
      }

      ++i;
    }

    //------

    CRGBA *colors = createImageColors(&xpm_data);

    xpm_data.data = new uint [xpm_data.width*xpm_data.height];

    for (j = 0; j < xpm_data.height; ++j) {
      if (i >= (int) num_strings) {
        CImage::errorMsg("Failed to read data");
        return false;
      }

      if (xpm_data.num_colors <= 256) {
        if (! readDataString(&xpm_data, strings[i], xpm_data.data, &pos))
          return false;
      }
      else {
        if (! readData24String(&xpm_data, colors, strings[i], xpm_data.data, &pos))
          return false;
      }

      ++i;
    }

    //------

    if (xpm_data.num_colors > 256) {
      if (xpm_data.transparent) {
        const CRGBA &color = colors[xpm_data.transparent_color];

        xpm_data.transparent_color = image->rgbaToPixel(color);
      }
    }

    //------

    image->setType(CFILE_TYPE_IMAGE_XPM);

    image->setDataSize(xpm_data.width, xpm_data.height);

    if (xpm_data.num_colors <= 256) {
      for (int i = 0; i < xpm_data.num_colors; ++i)
        image->addColor(colors[i]);

      image->setColorIndexData(xpm_data.data);

      if (xpm_data.transparent)
        image->setTransparentColor(xpm_data.transparent_color);
    }
    else
      image->setRGBAData(xpm_data.data);

    delete [] xpm_data.data;

    //------

    delete [] colors;

    delete [] xpm_data.colors;

    return true;
  }
  catch (...) {
    CImage::errorMsg("Failed read file");

    delete [] xpm_data.colors;

    return false;
  }
}

bool
CImageXPM::
readHeader(CFile *file, CImagePtr &image)
{
  CFileData *file_data = 0;

  file->rewind();

  try {
    CImageXPMData xpm_data;

    file_data = file->readAll();

    char *data = (char *) file_data->getData();

    //------

    int i = 0;

    //------

    if (! readHeader(data, &i)) {
      CImage::errorMsg("Failed to read header");
      return false;
    }

    if (! skipDcl(data, &i)) {
      CImage::errorMsg("Failed to read declarations");
      return false;
    }

    //------

    if (! readValues(data, &i, &xpm_data)) {
      CImage::errorMsg("Failed to read values");
      return false;
    }

    //------

    image->setType(CFILE_TYPE_IMAGE_XPM);

    image->setSize(xpm_data.width, xpm_data.height);
  }
  catch (...) {
    CImage::errorMsg("Failed to read XPM file");
    return false;
  }

  delete file_data;

  return true;
}

bool
CImageXPM::
readHeader(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '/' || data[*i + 1] != '*') {
    CImage::errorMsg("Failed find header start comment");
    return false;
  }

  *i += 2;

  CStrUtil::skipSpace(data, i);

  if (data[*i] != 'X' || data[*i + 1] != 'P' || data[*i + 2] != 'M') {
    CImage::errorMsg("Failed find \"XPM\" in comment");
    return false;
  }

  *i += 3;

  CStrUtil::skipSpace(data, i);

  if (data[*i] != '*' || data[*i + 1] != '/') {
    CImage::errorMsg("Failed find header end comment");
    return false;
  }

  *i += 2;

  CStrUtil::skipSpace(data, i);

  return true;
}

bool
CImageXPM::
skipDcl(const char *data, int *i)
{
  while (skipComment(data, i))
    ;

  //------

  /* Skip to Brace */

  while (data[*i] != '\0' && data[*i] != '{')
    (*i)++;

  if (data[*i] == '{')
    (*i)++;

  //------

  return true;
}

bool
CImageXPM::
readValues(const char *data, int *i, CImageXPMData *xpm_data)
{
  while (skipComment(data, i))
    ;

  //------

  // skip to start of string
  while (data[*i] != '\0' && data[*i] != '\"')
    (*i)++;

  //------

  char *str = readString(data, i);

  if (str == 0) {
    CImage::errorMsg("Failed value string");
    return false;
  }

  //------

  bool flag = readValuesString(xpm_data, str);

  //------

  delete [] str;

  //------

  return flag;
}

bool
CImageXPM::
readValuesString(CImageXPMData *xpm_data, const char *str)
{
  uint i = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->width)) {
    CImage::errorMsg("Failed to read width");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->height)) {
    CImage::errorMsg("Failed to read height");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->num_colors)) {
    CImage::errorMsg("Failed to read num colors");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->chars_per_pixel)) {
    CImage::errorMsg("Failed to read chars per pixel");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->x_hot))
    xpm_data->x_hot = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->y_hot))
    xpm_data->y_hot = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (strncmp(&str[i], "XPMEXT", 6) == 0)
    xpm_data->extension = true;

  return true;
}

bool
CImageXPM::
readColors(const char *data, int *i, CImageXPMData *xpm_data)
{
  xpm_data->colors = new CImageXPMColor [xpm_data->num_colors];

  //------

  while (skipComment(data, i))
    ;

  //------

  for (int j = 0; j < xpm_data->num_colors; ++j) {
    // skip to start of string
    while (data[*i] != '\0' && data[*i] != '\"')
      (*i)++;

    //------

    char *str = readString(data, i);

    if (str == 0) {
      CImage::errorMsg("Failed to read color string");
      return false;
    }

    //------

    bool flag = readColorString(xpm_data, str, &xpm_data->colors[j]);

    if (! flag) {
      CImage::errorMsg("Failed to parse color string '" + std::string(str) + "'");

      delete [] str;

      return false;
    }

    //------

    delete [] str;
  }

  //------

  return true;
}

bool
CImageXPM::
readColorString(CImageXPMData *xpm_data, const char *str, CImageXPMColor *color)
{
  int i = 0;

  //------

  color->name = std::string(&str[i], xpm_data->chars_per_pixel);

  i += xpm_data->chars_per_pixel;

  //------

  CStrUtil::skipSpace(str, &i);

  while (str[i] != '\0') {
    if      (str[i] == 'm' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->mono = CStrUtil::stripSpaces(std::string(&str[j], i - j));
    }
    else if (str[i] == 's' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->symb = CStrUtil::stripSpaces(std::string(&str[j], i - j));
    }
    else if (str[i] == 'g' && str[i + 1] == '4' && isspace(str[i + 2])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->grey4 = CStrUtil::stripSpaces(std::string(&str[j], i - j));
    }
    else if (str[i] == 'g' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->grey = CStrUtil::stripSpaces(std::string(&str[j], i - j));
    }
    else if (str[i] == 'c' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->color = CStrUtil::stripSpaces(std::string(&str[j], i - j));
    }
    else {
      CImage::errorMsg("Invalid color key '" + std::string(&str[i], 1) + "'");
      return false;
    }

    CStrUtil::skipSpace(str, &i);
  }

  return true;
}

void
CImageXPM::
skipToColorKey(const char *str, int *i)
{
  while (str[*i] != '\0') {
    if (isspace(str[*i]) && isColorKey(&str[*i + 1]))
      break;

    (*i)++;
  }
}

bool
CImageXPM::
isColorKey(const char *str)
{
  if      (str[0] == 'm' && isspace(str[1]))
    return true;
  else if (str[0] == 's' && isspace(str[1]))
    return true;
  else if (str[0] == 'g' && str[1] == '4' && isspace(str[2]))
    return true;
  else if (str[0] == 'g' && isspace(str[1]))
    return true;
  else if (str[0] == 'c' && isspace(str[1]))
    return true;

  CImage::errorMsg("Invalid color key '" + std::string(&str[0], 1) + "'");

  return false;
}

bool
CImageXPM::
readData(const char *data, int *i, CImageXPMData *xpm_data)
{
  xpm_data->data = new uint [xpm_data->width*xpm_data->height];

  //------

  while (skipComment(data, i))
    ;

  //------

  CRGBA *colors = createImageColors(xpm_data);

  //------

  int pos = 0;

  for (int y = 0; y < xpm_data->height; ++y) {
    // skip to start of string
    while (data[*i] != '\0' && data[*i] != '\"')
      (*i)++;

    //------

    char *str = readString(data, i);

    if (str == 0) {
      CImage::errorMsg("Failed to read color string");
      return false;
    }

    //------

    if (xpm_data->num_colors <= 256) {
      if (! readDataString(xpm_data, str, xpm_data->data, &pos))
        return false;
    }
    else {
      if (! readData24String(xpm_data, colors, str, xpm_data->data, &pos))
        return false;
    }

    //------

    delete [] str;
  }

  //------

  if (xpm_data->num_colors > 256) {
    if (xpm_data->transparent) {
      CRGBA *color = &colors[xpm_data->transparent_color];

      xpm_data->transparent_color = CImage::rgbaToPixel(*color);
    }
  }

  //------

  delete [] colors;

  //------

  return true;
}

bool
CImageXPM::
readDataString(CImageXPMData *xpm_data, const char *str, uint *data, int *pos)
{
  int len  = strlen(str);
  int elen = xpm_data->width*xpm_data->chars_per_pixel;

  if (len != elen) {
    CImage::errorMsg("Invalid string '" + std::string(str) + "' : (expected " +
                     std::to_string(elen) + " chars got " + std::to_string(len) + ")");
    return false;
  }

  //------

  int i = 0;

  if (xpm_data->chars_per_pixel == 1) {
    for (int x = 0; x < xpm_data->width; ++x) {
      int j = 0;

      for (j = 0; j < xpm_data->num_colors; ++j)
        if (str[i] == xpm_data->colors[j].name[0])
          break;

      if (j >= xpm_data->num_colors) {
        CImage::warnMsg("No color for symbol '" + std::string(&str[i], 1) + "'");
        j = 0;
      }

      data[(*pos)++] = j;

      //------

      ++i;
    }
  }
  else {
    for (int x = 0; x < xpm_data->width; ++x) {
      std::string name = std::string(&str[i], xpm_data->chars_per_pixel);

      int j = 0;

      for (j = 0; j < xpm_data->num_colors; ++j) {
        if (name == xpm_data->colors[j].name)
          break;
      }

      if (j >= xpm_data->num_colors) {
        CImage::warnMsg("No color for symbol '" + name + "'");
        j = 0;
      }

      data[(*pos)++] = j;

      //------

      i += xpm_data->chars_per_pixel;
    }
  }

  //------

  return true;
}

bool
CImageXPM::
readData24String(CImageXPMData *xpm_data, CRGBA *colors, const char *str, uint *data, int *pos)
{
  int len  = strlen(str);
  int elen = xpm_data->width*xpm_data->chars_per_pixel;

  if (len != elen) {
    CImage::errorMsg("Invalid string '" + std::string(str) + "' : (expected " +
                     std::to_string(elen) + " chars got " + std::to_string(len) + ")");
    return false;
  }

  //------

  int i = 0;

  for (int x = 0; x < xpm_data->width; ++x) {
    std::string name = std::string(&str[i], xpm_data->chars_per_pixel);

    int j = 0;

    for (j = 0; j < xpm_data->num_colors; ++j) {
      if (name == xpm_data->colors[j].name)
        break;
    }

    if (j >= xpm_data->num_colors) {
      CImage::warnMsg("No color for symbol '" + name + "'");
      j = 0;
    }

    data[(*pos)++] = CImage::rgbaToPixel(colors[j]);

    //------

    i += xpm_data->chars_per_pixel;
  }

  //------

  return true;
}

CRGBA *
CImageXPM::
createImageColors(CImageXPMData *xpm_data)
{
  CRGBA *colors = new CRGBA [xpm_data->num_colors];

  for (int i = 0; i < xpm_data->num_colors; ++i) {
    std::string name = xpm_data->colors[i].colorName();

    if (name != "") {
      if (CStrUtil::casecmp(name, "none") == 0) {
        colors[i].setRGBAI(0, 0, 0);

        xpm_data->transparent       = true;
        xpm_data->transparent_color = i;
      }
      else
        lookupColor(name, colors[i]);
    }
    else {
      CImage::warnMsg("No color data for color number " + std::to_string(i));

      colors[i].setRGBAI(0, 0, 0);
    }
  }

  return colors;
}

bool
CImageXPM::
skipComment(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '/' || data[*i + 1] != '*')
    return false;

  *i += 2;

  while (data[*i] != '\0' && (data[*i] != '*' || data[*i + 1] != '/'))
    (*i)++;

  if (data[*i] == '\0')
    return true;

  *i += 2;

  CStrUtil::skipSpace(data, i);

  return true;
}

char *
CImageXPM::
readString(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '\"')
    return 0;

  (*i)++;

  //------

  int j = *i;

  //------

  while (data[*i] != '\0' && data[*i] != '\"')
    (*i)++;

  if (data[*i] != '\"')
    return 0;

  char *str = CStrUtil::strndup(&data[j], *i - j);

  (*i)++;

  //------

  CStrUtil::skipSpace(data, i);

  if (data[*i] == ',') {
    (*i)++;

    CStrUtil::skipSpace(data, i);
  }

  //------

  return str;
}

bool
CImageXPM::
lookupColor(const std::string &name, CRGBA &color)
{
  if (name[0] != '#') {
    int r, g, b;

    if (CImageColorDef::getRGBI(name, &r, &g, &b))
      color.setRGBAI(r, g, b);
    else {
      CImage::warnMsg("Color name '" + name + "' not found");

      color.setRGBAI(0, 0, 0);
    }
  }
  else {
    char buffer[5];

    int len = name.size() - 1;

    if      (len == 12) {
      double rgb_scale = 1.0/65535.0;

      buffer[4] = '\0';

      uint r, g, b;

      strcpy(buffer, name.substr(1, 4).c_str()); sscanf(buffer, "%x", &r);
      strcpy(buffer, name.substr(5, 4).c_str()); sscanf(buffer, "%x", &g);
      strcpy(buffer, name.substr(9, 4).c_str()); sscanf(buffer, "%x", &b);

      color.setRGBA(r*rgb_scale, g*rgb_scale, b*rgb_scale);
    }
    else if (len == 6) {
      buffer[2] = '\0';

      uint r, g, b;

      strcpy(buffer, name.substr(1, 2).c_str()); sscanf(buffer, "%x", &r);
      strcpy(buffer, name.substr(3, 2).c_str()); sscanf(buffer, "%x", &g);
      strcpy(buffer, name.substr(5, 2).c_str()); sscanf(buffer, "%x", &b);

      color.setRGBAI(r, g, b);
    }
    else {
      CImage::warnMsg("Color name '" + name + "' not found");

      color.setRGBAI(0, 0, 0);
    }
  }

  return true;
}

//----------------------

bool
CImageXPM::
write(CFile *file, CImagePtr image)
{
  CImagePtr image1 = image;

  if (! image1->hasColormap()) {
    CImage::infoMsg("XPM Image Depth greater than 8 not supported - "
                    "Converting image to 256 colors");

    image1 = image1->dup();

    image1->convertToColorIndex();
  }

  //------

  char *colors_used;
  int   num_colors_used;

  getColorUsage(image1, &colors_used, &num_colors_used);

  //------

  std::string base = file->getBase();

  file->write("/* XPM */\n");
  file->write("\n");
  file->write("static const char *\n");
  file->write(base.c_str());
  file->write("_data[] = {\n");

  //------

  file->write("  /* width height num_colors chars_per_pixel x_hot y_hot */\n");

  xpm_chars_per_pixel_ = 1;

  int count = xpm_pixel_chars_.size();

  while (num_colors_used > count) {
    count *= xpm_pixel_chars_.size();

    xpm_chars_per_pixel_++;
  }

  file->write("  \"");
  file->write(CStrUtil::toString(image1->getWidth()));
  file->write(" ");
  file->write(CStrUtil::toString(image1->getHeight()));
  file->write(" ");
  file->write(CStrUtil::toString(num_colors_used));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_chars_per_pixel_));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_x_hot_));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_y_hot_));
  file->write("\",\n");
  file->write("\n");

  //------

  file->write("  /* colors */\n");

  int *pixel_map = new int [image1->getNumColors()];

  int j = 0;

  for (int i = 0; i < image1->getNumColors(); ++i) {
    if (! colors_used[i])
      continue;

    if (image1->isTransparent() && image1->getTransparentColor() == i) {
      pixel_map[i] = -1;
      continue;
    }

    pixel_map[i] = j;

    file->write("  \"");
    file->write(pixelToSymbol(j));
    file->write(" s symbol");
    file->write(CStrUtil::toString(j + 1));
    file->write(" c ");
    file->write(colorToString(image1, i));
    file->write(" m ");
    file->write(colorToMonoString(image1, i));
    file->write("\",\n");

    ++j;
  }

  if (image1->isTransparent())
    file->write("  \"  s mask c none\",\n");

  file->write("\n");

  //------

  file->write("  /* pixels */\n");

  int k = 0;

  for (uint i = 0; i < image1->getHeight(); ++i) {
    file->write("  \"");

    for (uint j = 0; j < image1->getWidth(); ++j, ++k) {
      int pixel = image1->getColorIndexPixel(k);

      file->write(pixelToSymbol(pixel_map[pixel]));
    }

    file->write("\",\n");
  }

  //------

  file->write("};\n");

  //------

  delete [] pixel_map;
  delete [] colors_used;

  return true;
}

void
CImageXPM::
getColorUsage(CImagePtr image, char **used, int *num_used)
{
  *used     = new char [image->getNumColors()];
  *num_used = 0;

  int i = 0;

  for (int i = 0; i < image->getNumColors(); ++i)
    (*used)[i] = 0;

  for (uint y = 0; y < image->getHeight(); ++y)
    for (uint x = 0; x < image->getWidth(); ++x, ++i) {
      int pixel = image->getColorIndexPixel(i);

      if (! (*used)[pixel])
        (*num_used)++;

      (*used)[pixel] |= 0x01;
    }
}

std::string
CImageXPM::
pixelToSymbol(int pixel)
{
  std::string pixel_string;

  if (pixel == -1) {
    for (int i = 0; i < xpm_chars_per_pixel_; ++i)
      pixel_string += ' ';

    return(pixel_string);
  }

  int pixel1 = pixel;

  for (int i = 0; i < xpm_chars_per_pixel_ - 1; ++i) {
    int pixel2 = pixel1 % xpm_pixel_chars_.size();

    pixel_string += xpm_pixel_chars_[pixel2];

    pixel1 /= xpm_pixel_chars_.size();
  }

  pixel_string += xpm_pixel_chars_[pixel1];

  return pixel_string;
}

std::string
CImageXPM::
colorToString(CImagePtr image, int color)
{
  uint r, g, b, a;

  image->getColorRGBAI(color, &r, &g, &b, &a);

  char color_string[32];

  sprintf(color_string, "#%02x%02x%02x", r & 0xFF, g & 0xFF, b & 0xFF);

  return std::string(color_string);
}

std::string
CImageXPM::
colorToMonoString(CImagePtr image, int color)
{
  uint r, g, b, a;

  image->getColorRGBAI(color, &r, &g, &b, &a);

  int gray = (r + g + b)/3;

  if (gray > 127)
    return "white";
  else
    return "black";
}
