TOP = $(abspath ../../)
TOOL_TOP = $(TOP)/tool

ifeq ($(OS),Windows_NT)
  DRIVE_LETTERS := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

  define check_drive_letter
  ifeq ($$(findstring $1:,$$(TOP)), $1:)
  TOP := $$(subst $1:,,$$(TOP))
  endif
  endef

  $(foreach letter,$(DRIVE_LETTERS),$(eval $(call check_drive_letter,$(letter))))
  BUSY_PRE = cmd /C $(TOOL_TOP)/busybox/busybox.exe
	subdirs := $(shell dir /ad /b)
else
	subdirs := $(shell ls -d1 */)
endif

NUM_CORES := 1
ifneq ($(OS),Windows_NT)
ifeq ($(shell uname),Darwin)
	NUM_CORES := $(shell sysctl -n hw.logicalcpu)
else
	NUM_CORES := $(shell nproc --all)
endif
endif

$(warning subdirs = $(subdirs))

.PHONY: all $(subdirs)

all: $(subdirs)

$(subdirs):
	+$(MAKE) -C $@ clean
	+$(MAKE) -C $@ -j$(NUM_CORES) EXEC_B8PLAYER=


info:
	$(MAKE) -C hello info

clean:
	@for dir in $(subdirs); do \
		echo "Cleaning $$dir"; \
		$(MAKE) -C $$dir clean; \
	done
