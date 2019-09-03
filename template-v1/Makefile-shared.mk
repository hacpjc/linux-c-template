#
# Sanity check
#
ifeq ($(MAKE),)
$(error Invalid symbol MAKE. Not possible!?)
endif

#
# INC_MAKEFILE_IN_DIR 
# Include a Makefile in a specified sub-directory. Pop an error if no directory is given.
# 1: The directory to include
#
define INC_MAKEFILE_IN_DIR
ifeq ($(1),)
$$(error Invalid symbol.)
else
include $(1)/Makefile.inc
endif
endef

#
# FORBID_EMPTY_SYMBOL 
# Check for a Makefile symbol. Pop an error if it's empty.
# 1: Symbol left value (name)
# 2: Symbol right value (value)
# 3: Additional error message to display.
#
define FORBID_EMPTY_SYMBOL
ifeq ($(2),)
$$(error Invalid symbol $(1). $(3))
endif
endef

#
# CMD_MAKE_ALL_DIR_NOSP
# Run make on specified directory list. Every directory path must not contain a space.
# 1: The directories to run "make"
# 2: The command/target to run at Makefile.
#
CMD_MAKE_ALL_DIR_NOSP = if [ ! "$(1)" = "" ]; then for d in $(1); do if [ ! -d "$$d" ]; then echo " * ERROR: $$d is not a directory" && exit 255; fi; echo "...Enter dir: $$d" && $(MAKE) -C $$d $(2) || exit 255; done; fi;

#
# CMD_MAKE_DIR_NOSP
# Run make on a single directory. The directory path must not contain a space.
# 1: The directory to run "make"
# 2: The command/target to run at Makefile.
#
CMD_MAKE_DIR_NOSP = if [ ! "$(1)" = "" ]; then if [ ! -d "$(1)" ]; then echo " * ERROR: $(1) is not a directory" && exit 255; fi; echo "...Enter dir: $(1)" && $(MAKE) -C $(1) $(2) || exit 255; fi;

#
# CMD_MAKE_LINUX
# A wrapper to run make on linux kernel (support cross compile)
# 1: Linux kernel source directory
# 2: This module directory
# 3: Kernel arch
# 4: Cross compiler prefix
# 5: Verbose level
# 6: Makefile target
#
CMD_MAKE_LINUX = if [ ! -d "$(1)" -o ! -s "$(1)/.config" ]; then echo " * ERROR: Plz make sure kernel config '$(1)/.config' is ready" && exit 255; else $(MAKE) -C "$(1)" M=$(2) $(if $(3),ARCH=$(3)) $(if $(4),CROSS_COMPILE=$(4)) $(if $(5),V=$(5)) $(6) ; fi;

#
# CMD_CHECK_DIR
# Check input path if it's an direcotry or not.
# 1: The directory path to check
#
CMD_CHECK_DIR = if [ ! -d "$(1)" ]; then echo " * ERROR: $(1) is not a directory" && exit 255; fi

#
# CMD_CHECK_REG_FILE
# Check input path if it's an regular file.
# 1: The file path to check
#
CMD_CHECK_REG_FILE = if [ ! -f "$(1)" ]; then echo " * ERROR: Expect $(1) to be a regular file, but it's not." && exit 255; fi

#
# CMD_FORBID_EMPTY_FILE
# Check if input file is empty.
# 1: The file path to check
#
CMD_FORBID_EMPTY_FILE = if [ ! -s "$(1)" ]; then echo " * ERROR: Expect $(1) to be a non-empty file, but it's not." && exit 255; fi

#
# CMD_FORBID_EMPTY_STR
# Compare two strings. Exit 255 if it's not equal.
# 1: input symbol (left value)
# 2: value of input symbol (right value)
# 3: Some more comment to display as error msg.
#
CMD_FORBID_EMPTY_STR = if [ "$(2)" = "" ]; then echo " * ERROR: Input string $(1) is empty." "$(3)"; exit 255; fi;

#
# CMD_CP_NOSP_R
# (Recursively) Copy file(s) into a directory. The file names should not contain space.
# 1: The files to copy
# 2: The directory to save.
#
CMD_CP_NOSP_R = if [ -d "$(2)" ]; then cp -prvf $(1) "$(2)"; else echo " * ERROR: $(2) is not a directory."; exit 255; fi

#
# CMD_CP_NOSP
# Copy specified file(s) into a directory. The file names should not contain a space.
# 1: The files to copy
# 2: The directory to save.
#
CMD_CP_NOSP = if [ -d "$(2)" ]; then cp -pvf $(1) "$(2)"; else echo " * ERROR: $(2) is not a directory."; exit 255; fi

#
# CMD_SOFTLINK_NOSP
# Link (soft) file(s) into a specified directory.
# 1: The input files.
# 2: The directory to be saved with new soft links.
#
CMD_SOFTLINK_NOSP = if [ -d "$(2)" ]; then ln -srf -v $(1) "$(2)"/; else  echo " * ERROR: $(2) is not a directory."; exit 255; fi

#
# CMD_PREP_DIR - Prepare an empty directory.
# Prepare a new directory.
# 1: The path of new directory
#
CMD_PREP_DIR = if [ "$(1)" = "" ]; then echo " * ERROR: Directory path is empty. Fix me."; exit 255; else mkdir -vp "$(1)" || exit 255; fi;

#
# CMD_CHECK_EXEC
# Check if an executable program can be found in local system.
# 1: The executable name (do not add prefix directory)
#
CMD_CHECK_EXEC = if [ "`which $(1)`" = "" ]; then echo " * ERROR: Local program $(1) does not exist." && exit 255; fi

#
# CMD_CC
# Run CC command with a more readable & short msg.
# 1: Output file
# 2: Complete CC command to run
#
ifeq ($(TMCFG_TC_DISABLE_COLOR_OUTPUT),y)
define CMD_CC
echo -e "\tCC $(1)"; ccout=`$(2) 2>&1`; ccret=$$?; if [ ! "$$ccout" = "" ]; then echo "$(2)" && echo "$$ccout" && echo "$$ccout" > $(1).log; else rm -f $(1).log; fi; test $$ccret -eq 0
endef
else
define CMD_CC
echo -e "\t\033[0;31mCC\033[0m $(1)"; ccout=`$(2) 2>&1`; ccret=$$?; if [ ! "$$ccout" = "" ]; then echo "$(2)" && echo "$$ccout" && echo "$$ccout" > $(1).log; else rm -f $(1).log; fi; test $$ccret -eq 0
endef
endif

#
# CMD_OBJCOPY_KEEP_DEBUG
# 1: objcopy exec path
# 2: Input files to run cmd
#
CMD_OBJCOPY_KEEP_DEBUG = for f in $(2); do echo -e "\tOBJCOPY $$f"; $(1) --only-keep-debug $$f $${f}.dbg || exit 255; done;

#
# CMD_STRIP
# 1: strip exec path and arguments
# 2: Input files to run strip
#
CMD_STRIP = for f in $(2); do echo -e "\tSTRIP $$f"; $(1) $$f || exit 255; done;

#
# Global Makefile (include the file to get all configured options in menuconfig).
#
-include $(PRJ_PATH_AUTOCONF)

ifeq ($(INTERNAL_ONE_TIME_EXPORT),)
ifneq ($(CONFIG_TC_EXTRA_LD_LIBRARY_PATH),)
export INTERNAL_ONE_TIME_EXPORT := y
export LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):)$(CONFIG_TC_EXTRA_LD_LIBRARY_PATH)
endif
endif

#;
