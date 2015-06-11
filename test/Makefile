CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/CImageConvert

SRC = \
CImageConvert.cpp

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
	$(RM) -f CImageConvert

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/CImageConvert: $(OBJS) $(LIB_DIR)/libCImageLib.a
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CImageConvert $(OBJS) $(LFLAGS) $(LIBS)
