#include <CImageLibI.h>

using std::map;
using std::cerr;
using std::endl;

CRGBA CImage::convertBg_ = CRGBA(1,1,1);

double CImage::convertAlphaTol_ = 0.1;

void
CImage::
convertToNColors(uint ncolors, ConvertMethod method)
{
  if (hasColormap()) {
    if (getNumColors() > (int) ncolors) {
      cerr << "convertToNColors not implemented for color map" << endl;
      return;
    }
  }

  uint num_bytes = size_.area();

  //----------

  // get map of colors and number of each

  map<uint,int> color_map;

  uint r, g, b, ind;

  uint *p1 = &data_[0];
  uint *p2 = &data_[num_bytes];

  for ( ; p1 < p2; ++p1) {
    pixelToRGBI(*p1, &r, &g, &b);

    ind = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);

    map<uint,int>::iterator pc = color_map.find(ind);

    if (pc != color_map.end())
      ++(*pc).second;
    else
      color_map[ind] = 1;
  }

  uint num_colors = color_map.size();

  if (CImageState::getDebug())
    cerr << "Num Colors " << num_colors << endl;

  //----------

  // shrink map to <= ncolors colors of the most used

  uint num = 0;

  while (num_colors > ncolors) {
    ++num;

    num_colors = 0;

    map<uint,int>::iterator pc1 = color_map.begin();
    map<uint,int>::iterator pc2 = color_map.end  ();

    for ( ; pc1 != pc2; ++pc1) {
      if ((*pc1).second <= (int) num)
        (*pc1).second = 0;
      else
        ++num_colors;
    }

    if (CImageState::getDebug())
      cerr << "Num Colors " << num_colors << endl;
  }

  //----------

  // assign colors

  double f = 1.0/255.0;

  int color_num = 0;

  map<uint,int>::iterator pc1 = color_map.begin();
  map<uint,int>::iterator pc2 = color_map.end  ();

  for ( ; pc1 != pc2; ++pc1) {
    if ((*pc1).second <= 0) {
      (*pc1).second = -1;
      continue;
    }

    int ind = (*pc1).first;

    int r = (ind >> 16) & 0xFF;
    int g = (ind >>  8) & 0xFF;
    int b = (ind >>  0) & 0xFF;

    addColor(r*f, g*f, b*f, 1);

    (*pc1).second = color_num++;
  }

  if (CImageState::getDebug())
    cerr << "Num Colors " << color_num << endl;

  //----------

  // assign indices to data

  if      (method == CONVERT_NEAREST_LOGICAL) {
    p1 = &data_[0];

    for ( ; p1 < p2; ++p1) {
      pixelToRGBI(*p1, &r, &g, &b);

      ind = ((r & 0xFF) << 16) | ((g & 0xFF) <<  8) | ((b & 0xFF) <<  0);

      map<uint,int>::iterator pc = color_map.find(ind);

      if ((*pc).second >= 0) {
        *p1 = (*pc).second;
        continue;
      }

      if ((*pc).second < -1) {
        *p1 = -((*pc).second + 2);
        continue;
      }

      int min_d = (1<<24);

      map<uint,int>::iterator pc1 = color_map.begin();
      map<uint,int>::iterator pc2 = color_map.end  ();

      for ( ; pc1 != pc2; ++pc1) {
        if ((*pc1).second < 0)
          continue;

        int ind = (*pc1).first;

        int r1 = (ind >> 16) & 0xFF;
        int g1 = (ind >>  8) & 0xFF;
        int b1 = (ind >>  0) & 0xFF;

        int d = (r - r1)*(r - r1) + (g - g1)*(g - g1) + (b - b1)*(b - b1);

        if (d < min_d) {
          (*pc).second = -((*pc1).second + 2);
          min_d        = d;
        }
      }

      *p1 = -((*pc).second + 2);
    }
  }
  else if (method == CONVERT_NEAREST_PHYSICAL) {
    int current = -1;

    p1 = &data_[0];

    for (uint x = 0; p1 < p2; ++p1, ++x) {
      if (x >= getWidth())
        x = 0;

      if (x == 0)
        current = -1;

      //----

      pixelToRGBI(*p1, &r, &g, &b);

      ind = ((r & 0xFF) << 16) | ((g & 0xFF) <<  8) | ((b & 0xFF) <<  0);

      map<uint,int>::iterator pc = color_map.find(ind);

      if ((*pc).second >= 0) {
        current = (*pc).second;
        *p1     = current;

        continue;
      }

      if (current >= 0) {
        *p1 = current;

        continue;
      }

      uint *pt = p1;

      ++p1; ++x;

      for ( ; p1 < p2; ++p1, ++x) {
        pixelToRGBI(*p1, &r, &g, &b);

        ind = ((r & 0xFF) << 16) | ((g & 0xFF) <<  8) | ((b & 0xFF) <<  0);

        map<uint,int>::iterator pc = color_map.find(ind);

        if ((*pc).second >= 0) {
          current = (*pc).second;
          *p1     = current;

          for (uint *p = pt; p < p1; ++p)
            *p = current;

          break;
        }
      }
    }
  }
  else
    assert(false);
}

void
CImage::
convertToColorIndex()
{
  double ralpha_tol = convertAlphaTol_;
  uint   alpha_tol  = 256*ralpha_tol;

  if (hasColormap())
    return;

  bool transparent = false;

  // TODO: handle transparency
  uint num_bytes = size_.area();

  uint num_colors;
  uint colors[256];

  uint cycle = 0;

  uint r_bits = 8;
  uint g_bits = 8;
  uint b_bits = 8;

  while (true) {
    if (CImageState::getDebug())
      cerr << "Bits R " << r_bits << " G " << g_bits << " B " << b_bits << endl;

    // count number of unique colors
    num_colors = 0;

    uint *p  = &data_[0];
    uint *p1 = &data_[num_bytes];

    while (p < p1) {
      uint pixel = *p;

      uint r, g, b, a;

      pixelToRGBAI(pixel, &r, &g, &b, &a);

      ++p;

      while (p < p1 && *p == pixel)
        ++p;

      if (a > alpha_tol) {
        uint k = 0;

        for ( ; k < num_colors; ++k)
          if (colors[k] == pixel)
            break;

        if (k >= num_colors) {
          if (num_colors >= 255)
            break;

          colors[num_colors++] = pixel;
        }
      }
      else
        transparent = true;
    }

    //----

    if (CImageState::getDebug())
      cerr << "Num Colors " << num_colors << endl;

    //----

    // we have enough colors (<= 255)
    if (p >= p1)
      break;

    //----

    // reduce colors one bit at a time in r, g, b order

    p = data_;

    uint error = 0;

    double r, g, b, a;

    if      (cycle == 2) {
      int max = (1 << b_bits) - 1;

      for (uint i = 0; i < num_bytes; ++i, ++p) {
        pixelToRGBA(*p, &r, &g, &b, &a);

        if (a > ralpha_tol) {
          int r1 = int(255*(r*a + convertBg_.getRed  ()*(1 - a)));
          int g1 = int(255*(g*a + convertBg_.getGreen()*(1 - a)));
          int b1 = int(255*(b*a + convertBg_.getBlue ()*(1 - a)));

          if (b1 < max)
            b1 += error;

          error = b1 & 1;

          *p = rgbaToPixelI(r1, g1, b1 >> 1, 255);
        }
        else
          *p = rgbaToPixelI(0, 0, 0, 0);
      }

      --b_bits;
    }
    else if (cycle == 1) {
      int max = (1 << g_bits) - 1;

      for (uint i = 0; i < num_bytes; ++i, ++p) {
        pixelToRGBA(*p, &r, &g, &b, &a);

        if (a > ralpha_tol) {
          int r1 = int(255*(r*a + convertBg_.getRed  ()*(1 - a)));
          int g1 = int(255*(g*a + convertBg_.getGreen()*(1 - a)));
          int b1 = int(255*(b*a + convertBg_.getBlue ()*(1 - a)));

          if (g1 < max)
            g1 += error;

          error = g1 & 1;

          *p = rgbaToPixelI(r1, g1 >> 1, b1, 255);
        }
        else
          *p = rgbaToPixelI(0, 0, 0, 0);
      }

      --g_bits;
    }
    else {
      int max = (1 << r_bits) - 1;

      for (uint i = 0; i < num_bytes; ++i, ++p) {
        pixelToRGBA(*p, &r, &g, &b, &a);

        if (a > ralpha_tol) {
          int r1 = int(255*(r*a + convertBg_.getRed  ()*(1 - a)));
          int g1 = int(255*(g*a + convertBg_.getGreen()*(1 - a)));
          int b1 = int(255*(b*a + convertBg_.getBlue ()*(1 - a)));

          if (r1 < max)
            r1 += error;

          error = r1 & 1;

          *p = rgbaToPixelI(r1 >> 1, g1, b1, 255);
        }
        else
          *p = rgbaToPixelI(0, 0, 0, 0);
      }

      --r_bits;
    }

    ++cycle;

    cycle %= 3;
  }

  if (CImageState::getDebug())
    cerr << "Bits R " << r_bits << " G " << g_bits << " B " << b_bits << endl;

  //------

  // set color indices in data

  uint *p  = &data_[0];
  uint *p1 = &data_[num_bytes];

  while (p < p1) {
    uint pixel = *p;

    uint r, g, b, a;

    pixelToRGBAI(pixel, &r, &g, &b, &a);

    uint k = 0;

    if (a > alpha_tol) {
      for ( ; k < num_colors; ++k)
        if (colors[k] == pixel)
          break;
    }
    else
      k = num_colors;

    *p++ = k;

    while (p < p1 && *p == pixel)
      *p++ = k;
  }

  // add colors
  deleteColors();

  double r_factor = 1.0/(((double) (1 << r_bits)) - 1.0);
  double g_factor = 1.0/(((double) (1 << g_bits)) - 1.0);
  double b_factor = 1.0/(((double) (1 << b_bits)) - 1.0);

  double r, g, b, a;

  for (uint i = 0; i < num_colors; ++i) {
    pixelToRGBA(colors[i], &r, &g, &b, &a);

    assert(a > ralpha_tol);

    int r1 = int(255*(r*a + convertBg_.getRed  ()*(1 - a)));
    int g1 = int(255*(g*a + convertBg_.getGreen()*(1 - a)));
    int b1 = int(255*(b*a + convertBg_.getBlue ()*(1 - a)));

    if (CImageState::getDebug())
      cerr << "Color " << i + 1 << " = R " << r1 << " G " << g1 << " B " << b1 << endl;

    addColor(r1*r_factor, g1*g_factor, b1*b_factor, 1.0);

    if (CImageState::getDebug())
      cerr << "Color " << i + 1 << " = R " << r1*r_factor <<
              " G " << g1*g_factor << " B " << b1*b_factor << endl;
  }

  if (transparent)
    addColor(0.0, 0.0, 0.0, 0.0);
}

void
CImage::
convertToRGB()
{
  if (! hasColormap())
    return;

  uint *p = data_;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++p) {
      uint pixel = *p;

      *p = rgbaToPixel(colors_[pixel]);
    }
  }

  deleteColors();
}
