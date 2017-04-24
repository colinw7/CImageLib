#include <CImageLib.h>
#include <CImageSIX.h>
#include <CFileParse.h>

namespace {

bool readParameter(CFileParse &parse, int &parameter) {
  std::string str;

  while (parse.isDigit())
    str += parse.readChar();

  if (str == "") {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }

  parameter = std::stoi(str);

  return true;
}

bool readParameters(CFileParse &parse, std::vector<int> &parameters) {
  while (! parse.eof()) {
    int parameter;

    if (! readParameter(parse, parameter))
      break;

    parameters.push_back(parameter);

    if (! parse.isChar(';'))
      break;

    parse.skipChar();
  }

  return parameters.size();
}

bool readSixData(CFileParse &parse, int &data) {
  int c = parse.readChar();

  if (c < 0x3f) {
    std::cerr << "Invalid data '" << c << "'" << std::endl;
    return false;
  }

  int c1 = c - 0x3f;

  if (c1 > 0x3f) {
    std::cerr << "Invalid data '" << c << "'" << std::endl;
    return false;
  }

  data = c1;

  return true;
}

}

bool
CImageSIX::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  CFileParse parse(file);

  if (! parse.isChar('\033'))
    return false;

  parse.skipChar();

  if (! parse.isChar('P'))
    return false;

  parse.skipChar();

  // semi-colon separated values ending in 'q'

  std::vector<int> parameters;

  while (! parse.eof()) {
    if (parse.isChar('q'))
      break;

    int p = parse.readChar();

    parameters.push_back(p);

    if (! parse.isChar(';'))
      break;

    parse.skipChar();
  }

  if (! parse.isChar('q'))
    return false;

  parse.skipChar();

  int w = 0, h = 0;

  std::vector<CRGBA> colors;

  while (! parse.eof()) {
    if      (parse.isChar('\"')) {
      parse.skipChar();

      std::vector<int> parameters;

      readParameters(parse, parameters);

      if (parameters.size() == 4) {
        w = parameters[2];
        h = parameters[3];
      }
    }
    else if (parse.isChar('#')) {
      parse.skipChar();

      std::vector<int> parameters;

      readParameters(parse, parameters);

      if (parameters.size() == 5) {
        int r = parameters[2];
        int g = parameters[3];
        int b = parameters[4];

        colors.push_back(CRGBA(r/255.0, g/255.0, b/255.0));
      }
    }
    else {
      break;
    }
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_SIX);

  image->setDataSize(w, h);

  for (const auto &color : colors)
    image->addColor(color);

  uint *data = new uint [w*h];

  //---

  int c = 0;
  int x = 0;
  int y = 0;

  while (! parse.eof()) {
    if (parse.isChar('\033')) {
      parse.skipChar();

      if (parse.isChar('\\'))
        break;

      //data.push_back('\033');

      return false;
    }
    // repeat data
    else if (parse.isChar('!')) {
      parse.skipChar();

      int parameter;

      if (! readParameter(parse, parameter))
        break;

      int d;

      if (! readSixData(parse, d))
        break;

      for (int i = 0; i < parameter; ++i) {
        for (int i = 0; i < 6; ++i) {
          bool b = d & (1<<i);

          if (b) data[x + (y + i)*w] = c;
        }

        ++x;
      }
    }
    // choose palette color
    else if (parse.isChar('#')) {
      parse.skipChar();

      int parameter;

      if (! readParameter(parse, parameter))
        break;

      c = parameter;
    }
    // carriage return
    else if (parse.isChar('$')) {
      parse.skipChar();

      x = 0;
    }
    // next line
    else if (parse.isChar('-')) {
      parse.skipChar();

      x = 0;

      y += 6;
    }
    // add data
    else {
      int d;

      if (! readSixData(parse, d))
        break;

      for (int i = 0; i < 6; ++i) {
        bool b = d & (1<<i);

        if (b) data[x + (y + i)*w] = c;
      }

      ++x;
    }
  }

  //------

  image->setColorIndexData(data);

  delete [] data;

  //------

  return true;
}

bool
CImageSIX::
readHeader(CFile *, CImagePtr &)
{
  return false;
}

bool
CImageSIX::
write(CFile *, CImagePtr)
{
  return false;
}
