PROJECT = libb8
OBJDIR = ./obj
TOP = $(abspath ../../../../)

ifeq ($(OS),Windows_NT)
  DRIVE_LETTERS := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

  define check_drive_letter
  ifeq ($$(findstring $1:,$$(TOP)), $1:)
  TOP := $$(subst $1:,,$$(TOP))
  endif
  endef

  $(foreach letter,$(DRIVE_LETTERS),$(eval $(call check_drive_letter,$(letter))))
endif

TOOL_TOP = $(TOP)/tool
B8LIB_TOP = $(TOP)/sdk/b8lib
B8LIB_INC = $(B8LIB_TOP)/include
INCLUDES += $(B8LIB_INC)
AFILE = $(B8LIB_TOP)/lib/$(PROJECT).a
GNUARM_TOP = $(TOP)/gnuarm

include	$(B8LIB_TOP)/makefile.top

OBJS = $(OBJDIR)/sys.o \
	$(OBJDIR)/os.o \
	$(OBJDIR)/misc.o \
	$(OBJDIR)/ppu.o \
	$(OBJDIR)/errno.o \
	$(OBJDIR)/semaphore.o \
	$(OBJDIR)/pthread.o \
	$(OBJDIR)/syscall.o \
	$(OBJDIR)/tmr.o \
	$(OBJDIR)/hif.o \
	$(OBJDIR)/sched.o

DEPS = $(OBJS:.o=.d)

.PHONY: all rebuild clean

all: $(AFILE)

rebuild: clean all

$(AFILE) : $(OBJS) Makefile
	@echo $(ESC_INFO)going to archive $(AFILE) $(ESC_RESET)
	$(AR) rc $@ $(OBJS)
	$(SIZE) --totals --common $@ | $(SED) -e s%$(B8LIB_TOP)/%% | $(TEE) $@.size
	$(ODUMP) -Mreg-names-std -d -C $@ > $@.lst

$(OBJDIR)/%.o: %.c
	$(call COMPILE_C)

clean:
	@$(RM) $(B8LIB_TOP)/lib/$(PROJECT).*
	@$(RM) $(OBJDIR)/*.o
	@$(RM) $(OBJDIR)/*.d

doc:
	cd $(B8LIB_TOP) && doxygen

# read dependency .d file (except clean target)
ifeq ($(filter $(MAKECMDGOALS), clean), )
ifeq ($(filter $(MAKECMDGOALS), mostlyclean), )
-include $(DEPS)
endif
endif
