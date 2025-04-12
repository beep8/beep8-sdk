PROJECT = libb8helper


OBJDIR = ./obj

TOP = $(abspath ../../../)

ifeq ($(OS),Windows_NT)
  DRIVE_LETTERS := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

  define check_drive_letter
  ifeq ($$(findstring $1:,$$(TOP)), $1:)
  TOP := $$(subst $1:,,$$(TOP))
  endif
  endef

  $(foreach letter,$(DRIVE_LETTERS),$(eval $(call check_drive_letter,$(letter))))
endif

TOOL_TOP  = $(TOP)/tool
B8LIB_TOP = $(TOP)/sdk/b8lib
B8LIB_INC = $(B8LIB_TOP)/include

B8HELPER_TOP = $(abspath ../)
B8HELPER_TOP = $(TOP)/sdk/b8helper
B8HELPER_INC = $(B8HELPER_TOP)/include

INCLUDES += $(B8LIB_INC)
INCLUDES += $(B8HELPER_INC)

AFILE = $(B8HELPER_TOP)/lib/$(PROJECT).a
GNUARM_TOP = $(TOP)/gnuarm

include	$(B8LIB_TOP)/makefile.top

C_SRC = $(wildcard *.c)
OBJS += $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRC))
CPP_SRC = $(wildcard *.cpp)
OBJS += $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRC))

PNGS     = $(wildcard ./data/import/*.png)
PNGS_CPP = $(patsubst %.png,%.png.cpp,$(PNGS))
PNGS_EXPORT_CPP = $(subst import,export,$(PNGS_CPP))
PNG_OBJS = $(patsubst %.cpp,%.o,$(PNGS_EXPORT_CPP))
OBJS += $(PNG_OBJS)

# png2c
PNG2C=$(TOOL_TOP)/png2c/$(OS)/$(HW)/png2c

DEPS = $(OBJS:.o=.d)

.PHONY: all rebuild clean

all: $(AFILE)

rebuild: clean all

$(AFILE) : $(OBJS) Makefile
	@echo $(ESC_INFO)going to archive $(AFILE) $(ESC_RESET)
	$(AR) rc $@ $(OBJS)
ifneq ($(OS),Windows_NT)
	$(SIZE) --totals --common $@ | $(SED) -e s%$(B8HELPER_TOP)/%% | $(TEE) $@.size
endif
	$(ODUMP) -Mreg-names-std -d -C $@ > $@.lst

$(OBJDIR)/%.o: %.c
	$(call COMPILE_C)

$(OBJDIR)/%.o: %.cpp
	$(call COMPILE_CPP)

${PNGS_EXPORT_CPP}: ${PNGS}
	$(MKDIR) $(dir $@)
	@echo $(ESC_INFO)
	@echo png2c $^ to $@
	$(PNG2C) $^ > $@
	@echo $(ESC_RESET)

.PHONY: info
info:
	@echo "TOUCH  = $(TOUCH)"
	@echo "BUSYBOX= $(BUSYBOX)"
	@echo "CCACHE = $(CCACHE)"

clean:
	@$(TOUCH) Makefile $(PNGS)
	@$(RM) $(B8HELPER_TOP)/lib/$(PROJECT).*
	@$(RM) $(OBJDIR)/*.o
	@$(RM) $(OBJDIR)/*.d

doc:
	cd $(B8HELPER_TOP) && doxygen

ifeq ($(filter $(MAKECMDGOALS), clean), )
ifeq ($(filter $(MAKECMDGOALS), mostlyclean), )
-include $(DEPS)
endif
endif
