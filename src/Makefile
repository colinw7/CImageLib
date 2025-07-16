CC = g++
AR = ar
RM = rm

CDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib

all: dirs $(LIB_DIR)/libCImageLib.a

dirs:
	@if [ ! -e ../obj ]; then mkdir ../obj; fi
	@if [ ! -e ../lib ]; then mkdir ../lib; fi
	@if [ ! -e ../bin ]; then mkdir ../bin; fi

CPP_SRC = \
CImage.cpp \
CImageAnim.cpp \
CImageBMP.cpp \
CImageColorDef.cpp \
CImageCombine.cpp \
CImageConvert.cpp \
CImageCopy.cpp \
CImageDiff.cpp \
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
CImageSIX.cpp \
CImageSGI.cpp \
CImageState.cpp \
CImageTGA.cpp \
CImageTIF.cpp \
CImageTile.cpp \
CImageUtil.cpp \
CImageWebP.cpp \
CImageXBM.cpp \
CImageXPM.cpp \
CImageXWD.cpp \

C_SRC = \
CLZW.c \

CPP_OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRC))

C_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SRC))

CPPFLAGS = \
$(CDEBUG) \
-DLARGEST_RECT \
-DIMAGE_PNG \
-DIMAGE_JPEG \
-std=c++17 \
-I$(INC_DIR) \
-I../../CFile/include \
-I../../CFileUtil/include \
-I../../CStrUtil/include \
-I../../COS/include \
-I../../CMath/include \
-I../../CUtil/include \
-I../../CUtil/include \
-I. \

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCImageLib.a

$(CPP_OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(C_OBJS): $(OBJ_DIR)/%.o: %.c
	gcc -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp .c

$(OBJ_DIR)/main.o: main.cpp
	$(CC) -c main.cpp -o $(OBJ_DIR)/main.o -c $(CPPFLAGS)

$(LIB_DIR)/libCImageLib.a: $(CPP_OBJS) $(C_OBJS)
	$(AR) crv $(LIB_DIR)/libCImageLib.a $(CPP_OBJS) $(C_OBJS)
