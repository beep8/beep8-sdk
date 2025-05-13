CFLAGS=-I. -g -Wall -Werror
OBJS=main.o
ENVPATH=

ifeq ($(OS), Windows_NT)
	ENVPATH = $(OS)/x86_64
	LIBS += -lpng -lz
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LDFLAGS=-static
		ENVPATH = linux/x86_64
		LIBS += -lpng -lz
	endif
	ifeq ($(UNAME_S), Darwin)
		ENVPATH = osx/x86_64
		LIBS += ./lib/$(ENVPATH)/libpng.a ./lib/$(ENVPATH)/libz.a
		ARCH := $(shell uname -m)
		ifeq ($(ARCH), arm64)
			CFLAGS += -target x86_64-apple-macos11
		endif
	endif
endif

TARGET = ./$(ENVPATH)/png2c

%.o: %.cpp
	g++ $(CFLAGS) $(INCS) -c -o $@ $<

$(TARGET): $(OBJS)
	mkdir -p $(ENVPATH)
	g++ $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(OS), Windows_NT)
	# List and copy necessary DLLs
	ldd $(TARGET) | grep '/mingw64/' | awk '{print $$3}' | xargs -I {} cp {} $(ENVPATH)/
endif
ifeq ($(UNAME_S), Darwin)
	cp $(TARGET) ./osx/arm64/
endif

clean:
	rm -rf $(TARGET) *.o

run:
	./$(TARGET) ./test_png/test.png > ./test_png/test.png.cpp
