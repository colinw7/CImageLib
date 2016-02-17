CC = g++
AR = ar
RM = rm

CDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(LIB_DIR)/libCImageLib.a

SRC = \
CImageAnim.cpp \
CImageBMP.cpp \
CImageColorDef.cpp \
CImageCombine.cpp \
CImageConvert.cpp \
CImageCopy.cpp \
CImage.cpp \
CImageDraw.cpp \
CImageFile.cpp \
CImageFilter.cpp \
CImageFlip.cpp \
CImageFloodFill.cpp \
CImageGen.cpp \
CImageGIF.cpp \
CImageGradient.cpp \
CImageGrayScale.cpp \
CImageICO.cpp \
CImageIFF.cpp \
CImageInvert.cpp \
CImageJPG.cpp \
CImageLineArt.cpp \
CImageMask.cpp \
CImageMerge.cpp \
CImageMgr.cpp \
CImagePCX.cpp \
CImagePNG.cpp \
CImagePPM.cpp \
CImageProcess.cpp \
CImagePSP.cpp \
CImageResize.cpp \
CImageRotate.cpp \
CImageScroll.cpp \
CImageSGI.cpp \
CImageState.cpp \
CImageTGA.cpp \
CImageTIF.cpp \
CImageTile.cpp \
CImageUtil.cpp \
CImageXBM.cpp \
CImageXPM.cpp \
CImageXWD.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
$(CDEBUG) \
-DLARGEST_RECT \
-DIMAGE_PNG \
-DIMAGE_JPEG \
-std=c++11 \
-I$(INC_DIR) \
-I../../CFile/include \
-I../../CStrUtil/include \
-I../../COS/include \
-I../../CMath/include \
-I../../CUtil/include \
-I../../CUtil/include \
-I. \

LFLAGS = -L$(LIB_DIR)

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCImageLib.a

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp

$(OBJ_DIR)/main.o: main.cpp
	$(CC) -c main.cpp -o $(OBJ_DIR)/main.o -c $(CPPFLAGS)

$(LIB_DIR)/libCImageLib.a: $(OBJS)
	$(AR) crv $(LIB_DIR)/libCImageLib.a $(OBJS)
