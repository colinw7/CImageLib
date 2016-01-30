#include <CImageLibI.h>
#include <CImageColorDefP.h>

#include <cstring>

bool
CImageColorDef::
getRGB(const std::string &name, double *r, double *g, double *b)
{
  int ri, gi, bi;

  if (! getRGBI(name, &ri, &gi, &bi))
    return false;

  double rgb_scale = 1.0/255.0;

  *r = ri*rgb_scale;
  *g = gi*rgb_scale;
  *b = bi*rgb_scale;

  return true;
}

bool
CImageColorDef::
getRGBI(const std::string &name, int *r, int *g, int *b)
{
  int i;

  std::string lname = CStrUtil::toLower(name);

  const char *name1 = lname.c_str();

  for (i = 0; color_def_data[i].name != 0; ++i)
    if (strcmp(color_def_data[i].name, name1) == 0)
      break;

  if (color_def_data[i].name == 0)
    return false;

  *r = color_def_data[i].r;
  *g = color_def_data[i].g;
  *b = color_def_data[i].b;

  return true;
}
