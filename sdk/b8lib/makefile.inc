BUILDTOP ?= $(TOPDIR)/build

GCCVER = gcc11.3

# HW : x86_64 or arm64
ifeq ($(OS),Windows_NT)
	HW = x86_64
else
	HW = $(shell uname -m)
endif

ifeq ($(OS),Windows_NT)
  BUSY_PRE = cmd /C $(TOOL_TOP)/busybox/busybox.exe
  GCCVER = gcc10.3
else ifeq ($(shell uname),Linux)
	OS = linux
  GCCVER = gcc10.2
	EXEC_B8PLAYER = $(TOOL_TOP)/beep8_app_player/Beep8AppPlayer-linux-x64/Beep8AppPlayer --args -b $(B8) $(SPAGE)
else ifeq ($(shell uname),Darwin)
	OS = osx
	GCCVER = gcc11.3
	EXEC_B8PLAYER = open -a Beep8AppPlayer --args -b $(B8) $(SPAGE)
endif

CP    = $(BUSY_PRE) cp -rpf
TOUCH = $(BUSY_PRE) touch
CAT   = $(BUSY_PRE) cat
WHICH = $(BUSY_PRE) which
SED   = $(BUSY_PRE) sed
RM    = $(BUSY_PRE) rm -rf
MKDIR = $(BUSY_PRE) mkdir -p
TEE   = $(BUSY_PRE) tee
LS    = @$(BUSY_PRE) ls -al

ifneq ($(shell $(WHICH) python3 ),)
PYTHON = python3
else ifneq ($(shell $(WHICH) python ),)
PYTHON = python
else
PYTHON = py
endif

ifeq ($(OS),Windows_NT)
EXT:=.exe
endif

ifeq ($(strip $(CCACHE)),)
  CCACHE := $(TOOL_TOP)/ccache/$(OS)/$(HW)/ccache$(EXT)
    ifeq ($(wildcard $(CCACHE)),)
      CCACHE := $(shell $(WHICH) ccache)
      ifeq ($(wildcard $(CCACHE)),)
        CCACHE :=
      endif
  endif
endif

ARMGCC = arm-none-eabi-
CROSS = $(GNUARM_TOP)/$(OS)/$(GCCVER)/$(HW)/bin/$(ARMGCC)

CC    = $(CCACHE) $(CROSS)gcc
CXX   = $(CCACHE) $(CROSS)g++
AS    = $(CROSS)gcc
LD    = $(CROSS)g++
NM    = $(CROSS)nm
SIZE  = $(CROSS)size
AR    = $(CROSS)ar
OCOPY = $(CROSS)objcopy
ODUMP = $(CROSS)objdump

MAKEOPT ?= -j6
export Q := @
#export Q :=

ifeq ($(OS),Windows_NT)
ESC_RESET   = [0m
ESC_INFO    = [36m
ESC_ERROR   = [31m
ESC_ASM     = [32m
ESC_COMPILE = [33m
else
ESC_RESET   = "\x1b[0m"
ESC_INFO    = "\x1b[1;36m"
ESC_ERROR   = "\x1b[1;31m"
ESC_ASM     = "\x1b[1;32m"
ESC_COMPILE = "\x1b[1;33m"
endif

# -O0 or -O1
OPTIMIZE	:= -O2

CWARN = -Wall
CWARN += -Wextra

# The -Wno-missing-field-initializers option instructs the compiler
# to disable warnings related to missing field initializers during 
# the initialization of structures (or unions).
# Normally, C and C++ compilers require that all fields of a structure
# be initialized during initialization. However, in some code,
# it may be intentional to initialize only some fields and leave others uninitialized.
# In such cases, the compiler generates a warning,
# but you can use the -Wno-missing-field-initializers option to suppress this warning.
CWARN += -Wno-missing-field-initializers

# The -Wno-deprecated-copy option instructs the compiler to disable warnings 
# related to the use of deprecated copy constructors and copy assignment operators.
# In C++ programming, copy-related operations (such as creating 
# copies of objects and assignment) are important for writing efficient code. 
# Starting from C++11, the C++ language specification introduced the concept 
# of "move semantics" and provided ways to achieve efficient object copying. 
# As a result, the use of old copy constructors and copy assignment operators
# has been deprecated.
# By using the -Wno-deprecated-copy option, the compiler suppresses warnings 
# related to deprecated copy constructors and copy assignment operators,
# and these warnings will not be displayed when compiling the code.
CXXWARN += -Wno-deprecated-copy

CFLAGS += -fmax-errors=5

CFLAGS += -I. $(addprefix -I,$(INCLUDES))

# The -g3 option generates the highest level of debugging information for the program.
# In addition to the basic debugging symbols provided by -g, it includes information
# about macros, allowing the debugger to inspect macros used in the code.
# This can be particularly useful for troubleshooting complex codebases with many macros.
# Note that enabling -g3 will increase the size of the output binary, so it is typically
# used during development and debugging, but omitted in production builds.
#CFLAGS += -g3

CFLAGS += $(OPTIMIZE)

# Built-in functions are typically functions provided by the compiler for advanced optimizations.
# These functions are often more efficient than standard library functions and may take advantage
# of specific hardware features. 
# Examples of built-in functions include memory copy functions like memcpy,
# string comparison functions like strcmp, and mathematical functions like sqrt.
CFLAGS += -fno-builtin

# The -ffunction-sections option instructs the GCC (GNU Compiler Collection)
# compiler to place code into separate sections for each function.
# Normally, the compiler places code into a single section and ignores function boundaries.
# However, when you specify the -ffunction-sections option, the compiler places each function into its own section.
# This makes it easier to remove specific functions or avoid loading them at runtime. Additionally,
# it allows the linker (typically GNU ld) to link only the necessary functions.
# The main advantages of using the -ffunction-sections option include:
#   Reduced File Size:
#     By excluding unused functions, you can reduce the size of the executable file.
#     This is particularly useful in embedded systems or resource-constrained environments.
#   Reduced Runtime Memory Usage:
#     Unnecessary functions do not need to be loaded into memory at runtime,
#     reducing the program's memory footprint.
#   Support for Advanced Optimization:
#     Splitting code into sections for each function can make it easier
#     for the compiler to perform more advanced optimizations.
CFLAGS += -ffunction-sections

# The -fdata-sections option instructs the GCC (GNU Compiler Collection) compiler
# to place data (variables and constants) into sections separately.
# When this option is specified, the compiler arranges data into individual sections,
# effectively subdividing the data sections.
CFLAGS += -fdata-sections

# By using the -fno-common option, the compiler disables the common block,
# and global variables declared in different source files will have their own memory storage.
# This can help avoid variable conflicts and enhance program stability.
CFLAGS += -fno-common
CFLAGS += $(CWARN)

# Specifically, when you use the -MD option, for each source file being compiled,
# the compiler generates a list of other files (header files or source files) that the file depends on.
# These dependency files typically have a .d extension and are generated in the same directory as the source file.
CFLAGS += -MD

# The -MP option is an option in the GCC (GNU Compiler Collection) compiler 
# that is used to support the build process in Makefiles.
# This option makes it easier to manage target dependencies in a Makefile.
CFLAGS += -MP

CFLAGS += -Wshadow -Wundef

CFLAGS += -fno-strength-reduce

# Disable GCC strict-aliasing optimizations to prevent undefined behavior when type-punning via pointer casts.
# Removing this flag is dangerous and not recommended.
CFLAGS += -fno-strict-aliasing

# The -fomit-frame-pointer option is a compiler option used with the GCC (GNU Compiler Collection)
# to instruct the compiler to omit the frame pointer for function calls. 
# The frame pointer is used for accessing local variables 
# and manipulating the stack frame within a function.
# When this option is specified, the compiler typically generates code that doesn't use 
# the frame pointer and operates the function's stack frame more efficiently.
# Omitting the frame pointer often results in faster function calls. However,
# it's important to note that debugging information generation and stack 
# trace analysis can become more challenging with this option enabled.
CFLAGS += -fomit-frame-pointer
#CFLAGS += -fno-omit-frame-pointer

# The -nostartfiles option is a compiler option in GCC (GNU Compiler Collection)
# that disables the inclusion of the standard startup files (typically crt0.o or crt1.o)
# that are normally linked at the beginning of a C/C++ program's execution.
CFLAGS += -nostartfiles

# The -mno-thumb-interwork option is a compiler option in GCC (GNU Compiler Collection)
# that disables interworking of function calls between Thumb mode and ARM mode.
# The virtual CPU of BEEP-8 does not have Thumb capability, so please always specify -mno-thumb-interwork.
CFLAGS += -mno-thumb-interwork

# The virtual CPU of BEEP-8 only has ARM mode capabilities, so please always specify -marm.
CFLAGS += -marm

# The virtual CPU of BEEP-8 does not have floating-point capabilities, so please always specify `-mfloat-abi=soft`.
CFLAGS += -mfloat-abi=soft

# Since the virtual CPU of BEEP-8 adopts ARM architecture version 4 (1992), please make sure to specify -march=armv4.
CFLAGS += -march=armv4

CFLAGS += -mcpu=strongarm
CFLAGS += -fno-lto

# In the BEEP-8 environment, you can utilize the C++20 standard.
CXXFLAGS += -std=c++2a

# Disables thread-safe initialization of static variables.
# This improves performance but is not safe in multi-threaded environments,
# so developers must ensure proper synchronization if static variables are accessed across threads.
CXXFLAGS += -fno-threadsafe-statics

# The C++ exception mechanism is disabled by default.
# However, exceptions thrown by standard libraries such as std::vector
# will trigger std::terminate(), which outputs exception details to stderr,
# allowing developers to be aware of the exception that occurred.
CXXFLAGS += -fno-exceptions

CXXFLAGS += $(CXXWARN)

# The `-specs=nano.specs` option is part of the GCC (GNU Compiler Collection) linker options. 
# This option is designed to optimize and configure executable files,
# particularly in resource-constrained environments like embedded systems,
# with the goal of reducing their size.
#
# Here are key features and explanations of the `-specs=nano.specs` option:
#
# 1. **Size Optimization**: When you use the `-specs=nano.specs` option,
#   it triggers size optimization during the compilation and linking processes.
#   This optimization reduces the size of executable files,
#   helping to conserve memory and storage resources.
#
# 2. **libnano Library**: This option utilizes a new C library called `libnano`.
#   `libnano` is a size-optimized version of the standard C library (`libc`) and is provided as an alternative. 
#   It reduces unnecessary features and data to minimize the size of executable files.
#
# 3. **Default Setting Changes**: Using the `-specs=nano.specs` option may result in changes to some default settings.
#   For example, it might disable support for floating-point numbers,
#   which can reduce the size of executable files but may impact programs that rely on floating-point operations.
#
# 4. **Customization**: Users have the flexibility to customize the `-specs=nano.specs` option.
#   This allows adjustments to the level of optimization and linker settings
#   to match specific resource constraints and requirements.
#
# The `-specs=nano.specs` option is particularly useful in projects where executable size reduction is critical,
# such as in embedded systems or resource-constrained environments.
# However, since certain features may be limited depending on the settings,
# it should be used with careful consideration.
# Choosing the right configuration to align with project requirements and available resources is essential.
# LDFLAGS	+= -specs=nano.specs

# Unnecessary code and data sections are removed, optimizing the size of the executable file.
# If undefined symbols are found, the linking process is interrupted, an error is reported, and the integrity of the program is ensured.
LDFLAGS	+= -Wl,--gc-sections,--no-undefined

LDFLAGS	+= --static
LDFLAGS	+= -Wl,-Map=$(OBJDIR)/$(PROJECT).map
LDFLAGS	+= -marm
LDFLAGS	+= -fno-threadsafe-statics
LDFLAGS	+= -fno-lto

define COMPILE_C
	$(Q)$(MKDIR) $(dir $@)
	@echo $(ESC_COMPILE)c compile $(abspath $<) $(ESC_RESET)
	$(Q)$(CC) -c $(CFLAGS) -Wstrict-prototypes $< -MMD -MP -MT $@ -o $@
endef

define COMPILE_CPP
	$(Q)$(MKDIR) $(dir $@)
	@echo $(ESC_COMPILE)c++ compile $(abspath $<) $(ESC_RESET)
	$(Q)$(CXX) -c $(CFLAGS) $(CXXFLAGS) $< -MMD -MP -MT $@ -o $@
endef

define ASM
	$(Q)$(MKDIR) $(dir $@)
	@echo $(ESC_ASM)assemble $(abspath $<) $(ESC_RESET)
	$(Q)$(AS) -c $(CFLAGS) $< -MMD -MP -MT $@ -o $@
endef
