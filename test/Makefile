CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

#PROG = CImageColorReplace
PROG = CImageConvert
#PROG = CImageCursorCombine
#PROG = CImageCursor
#PROG = CImageGIFAnim
#PROG = CImageHTMLGen
#PROG = CImageInfo
#PROG = CImageIsometric
#PROG = CImageNColors
#PROG = CImageProcess
#PROG = CImageResize
#PROG = CImageThumbnails
#PROG = CImageUntile

all: $(BIN_DIR)/$(PROG)

SRC = \
$(PROG).cpp

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-std=c++11 \
-I$(INC_DIR) \
-I. \
-I../../CImageLib/include \
-I../../CFile/include \
-I../../CMath/include \
-I../../CStrUtil/include \
-I../../CUtil/include \

LFLAGS = \
-L$(LIB_DIR) \
-L../../CImageLib/lib \
-L../../CFile/lib \
-L../../CStrUtil/lib \
-L../../COS/lib \
-L../../CRGBName/lib \

LIBS = \
-lCImageLib -lCFile -lCStrUtil \
-lCOS -lCRGBName \
-lpng -ljpeg

clean:
	$(RM) -f *.o
	$(RM) -f $(PROG)

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/$(PROG): $(OBJS) $(LIB_DIR)/libCImageLib.a
	$(CC) $(LDEBUG) -o $(BIN_DIR)/$(PROG) $(OBJS) $(LFLAGS) $(LIBS)
