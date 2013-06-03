#ifndef XWD_FILE_H
#define XWD_FILE_H

#define StaticGray  0
#define GrayScale   1
#define StaticColor 2
#define PseudoColor 3
#define TrueColor   4
#define DirectColor 5

#define XYBitmap 0
#define XYPixmap 1
#define ZPixmap  2

#define LSBFirst 0
#define MSBFirst 1

#define XWD_FILE_VERSION 7

#ifdef IMAGE_WORD64
# define sz_XWDheader 104
#else
# define sz_XWDheader 100
#endif

#define sz_XWDColor 12

struct XWDFileHeader {
  CIMAGE_INT32 header_size      :32; /* Size of the entire header (bytes) */
  CIMAGE_INT32 file_version     :32; /* XWD_FILE_VERSION */
  CIMAGE_INT32 pixmap_format    :32; /* Pixmap format */
  CIMAGE_INT32 pixmap_depth     :32; /* Pixmap depth */
  CIMAGE_INT32 pixmap_width     :32; /* Pixmap width */
  CIMAGE_INT32 pixmap_height    :32; /* Pixmap height */
  CIMAGE_INT32 xoffset          :32; /* Bitmap x offset */
  CIMAGE_INT32 byte_order       :32; /* MSBFirst, LSBFirst */
  CIMAGE_INT32 bitmap_unit      :32; /* Bitmap unit */
  CIMAGE_INT32 bitmap_bit_order :32; /* MSBFirst, LSBFirst */
  CIMAGE_INT32 bitmap_pad       :32; /* Bitmap scanline pad */
  CIMAGE_INT32 bits_per_pixel   :32; /* Bits per pixel */
  CIMAGE_INT32 bytes_per_line   :32; /* Bytes per scanline */
  CIMAGE_INT32 visual_class     :32; /* Class of colormap */
  CIMAGE_INT32 r_mask           :32; /* Z red mask */
  CIMAGE_INT32 g_mask           :32; /* Z green mask */
  CIMAGE_INT32 b_mask           :32; /* Z blue mask */
  CIMAGE_INT32 bits_per_rgb     :32; /* Log2 of distinct color values */
  CIMAGE_INT32 colormap_entries :32; /* Number of entries in colormap */
  CIMAGE_INT32 ncolors          :32; /* Number of Color structures */
  CIMAGE_INT32 window_width     :32; /* Window width */
  CIMAGE_INT32 window_height    :32; /* Window height */
  CIMAGE_INT32 window_x         :32; /* Window upper left X coordinate */
  CIMAGE_INT32 window_y         :32; /* Window upper left Y coordinate */
  CIMAGE_INT32 window_bdrwidth  :32; /* Window border width */
#ifdef IMAGE_WORD64
  CIMAGE_INT32 header_end       :32; /* Pad to fill out word */
#endif
};

struct XWDColor {
  CIMAGE_INT32 pixel :32;
  CIMAGE_INT16 r:16;
  CIMAGE_INT16 g:16;
  CIMAGE_INT16 b:16;
  CIMAGE_INT8  flags;
  CIMAGE_INT8  pad;
};

#endif
