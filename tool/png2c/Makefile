CFLAGS = -I. -g -Wall -Werror
OBJS = main.o
ENVPATH =

ifeq ($(OS), Windows_NT)
	ENVPATH = Windows_NT/x86_64
	LIBS += -lpng -lz
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LDFLAGS = -static
		ENVPATH = linux/x86_64
		LIBS += -lpng -lz
	endif
	ifeq ($(UNAME_S), Darwin)
		# 固定でIntelバイナリを出力
		ENVPATH = osx/x86_64
		CFLAGS += -arch x86_64
		LDFLAGS += -arch x86_64
		LIBS += ./lib/osx/x86_64/libpng.a ./lib/osx/x86_64/libz.a
	endif
endif

TARGET = ./$(ENVPATH)/png2c

%.o: %.cpp
	g++ $(CFLAGS) $(INCS) -c -o $@ $<

$(TARGET): $(OBJS)
	mkdir -p $(ENVPATH)
	g++ $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

ifeq ($(UNAME_S), Darwin)
	mkdir -p ./osx/arm64
	cp $(TARGET) ./osx/arm64/png2c
endif

ifeq ($(OS), Windows_NT)
	ldd $(TARGET) | grep '/mingw64/' | awk '{print $$3}' | xargs -I {} cp {} $(ENVPATH)/
endif

clean:
	rm -rf $(TARGET) *.o ./osx/arm64/png2c

run:
	./$(TARGET) ./test_png/test.png > ./test_png/test.png.cpp
