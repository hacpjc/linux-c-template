#
# Prerequisites
#
include $(PRJ_MK_SHARED)

#
# tc
#
export my-cc := $(CONFIG_TC_CC)
export my-ar := $(CONFIG_TC_AR)
export my-ranlib := $(CONFIG_TC_RANLIB)
export my-strip := $(CONFIG_TC_STRIP)
export my-objdump := $(CONFIG_TC_OBJDUMP)
export my-objcopy := $(CONFIG_TC_OBJCOPY)
export my-strip-enable := $(CONFIG_TC_RUN_STRIP)

#
# cflags, ldflags
#
my-cflags-y :=
my-cflags-y += $(CONFIG_TC_EXTRA_CFLAGS)
my-cflags-y := $(filter-out "",$(my-cflags-y))

my-ldflags-y :=
my-ldflags-y += $(CONFIG_TC_EXTRA_LDFLAGS)
my-ldflags-y := $(filter-out "",$(my-ldflags-y))

CFLAGS += $(my-cflags-y)
LDFLAGS += $(my-ldflags-y)

#
# all
# 
.PHONY: all
all: lib_build lib_install

#
# liburcu
#
urcu-pfx := $(CURDIR)/.stage
urcu-version := 0.10.1
urcu-tarball := userspace-rcu-$(urcu-version).tar.bz2
urcu-folder := userspace-rcu-$(urcu-version)

urcu-cflags := $(CFLAGS)
urcu-ldflags := $(LDFLAGS)

.PHONY: lib_build
lib_build: $(urcu-folder)/Makefile
	@test -d $(urcu-folder)
	$(MAKE) -C $(urcu-folder)
	$(MAKE) -C $(urcu-folder) install

$(urcu-folder)/Makefile: $(urcu-folder)
	(cd "$(urcu-folder)" && CC=$(my-cc) CFLAGS="$(urcu-cflags)" LDFLAGS="$(urcu-ldflags)" AR="$(my-ar)" RANLIB="$(my-ranlib)" STRIP="$(my-strip)" ./configure --prefix="$(urcu-pfx)" --host=$(shell $(my-cc) -dumpmachine) --disable-shared --enable-static)

$(urcu-folder): $(urcu-tarball)
	tar -jxvf $(urcu-tarball) -C $(CURDIR)
	touch $(urcu-folder)

.PHONY: lib_install
lib_install:
	@$(call CMD_PREP_DIR,$(PRJ_DIR_STAGE_INC))
	@$(call CMD_CP_NOSP_R,$(urcu-pfx)/include/*,$(PRJ_DIR_STAGE_INC))
	@$(call CMD_PREP_DIR,$(PRJ_DIR_STAGE_LIB))
	@$(call CMD_CP_NOSP_R,$(urcu-pfx)/lib/*,$(PRJ_DIR_STAGE_LIB))

.PHONY: lib_clean
lib_clean:
	-@$(MAKE) -C $(urcu-folder) clean
	rm -rvf $(urcu-pfx)

.PHONY: lib_distclean
lib_distclean: lib_clean
	rm -rvf $(urcu-folder)

.PHONY: clean
clean: lib_clean

.PHONY: distclean
distclean: lib_distclean

.PHONY: install
install:

.PHONY: release
release:

#;
