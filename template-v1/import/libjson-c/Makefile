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
# libjson-c
#
json-c-pfx := $(CURDIR)/.stage
json-c-version := 0.13.1
json-c-tarball := json-c-$(json-c-version).tar.gz
json-c-folder := json-c-$(json-c-version)

json-c-cflags := $(CFLAGS)
json-c-ldflags := $(LDFLAGS)

.PHONY: lib_build
lib_build: $(json-c-folder)/Makefile
	@test -d $(json-c-folder)
	$(MAKE) -C $(json-c-folder)
	$(MAKE) -C $(json-c-folder) install

$(json-c-folder)/Makefile: $(json-c-folder)
	(cd "$(json-c-folder)" && CC=$(my-cc) CFLAGS="$(json-c-cflags)" LDFLAGS="$(json-c-ldflags)" AR="$(my-ar)" RANLIB="$(my-ranlib)" STRIP="$(my-strip)" ./configure --prefix="$(json-c-pfx)" --host=$(shell $(my-cc) -dumpmachine) --disable-shared --enable-static)

$(json-c-folder): $(json-c-tarball)
	tar -zxvf $(json-c-tarball) -C $(CURDIR)
	touch $(json-c-folder)

.PHONY: lib_install
lib_install:
	@$(call CMD_PREP_DIR,$(PRJ_DIR_STAGE_INC))
	@$(call CMD_CP_NOSP_R,$(json-c-pfx)/include/*,$(PRJ_DIR_STAGE_INC))
	@$(call CMD_PREP_DIR,$(PRJ_DIR_STAGE_LIB))
	@$(call CMD_CP_NOSP_R,$(json-c-pfx)/lib/*,$(PRJ_DIR_STAGE_LIB))

.PHONY: lib_clean
lib_clean:
	-@$(MAKE) -C $(json-c-folder) clean
	rm -rvf $(json-c-pfx)

.PHONY: lib_distclean
lib_distclean: lib_clean
	rm -rvf $(json-c-folder)

.PHONY: clean
clean: lib_clean

.PHONY: distclean
distclean: lib_distclean

.PHONY: install
install:

.PHONY: release
release:

#;
