#
# Top level makefile: Plz execute all make commands here.
#

#
# Prerequisites
#
export PRJ_MK_SHARED := $(CURDIR)/Makefile-shared.mk
include $(PRJ_MK_SHARED)

#
# Root directory
#
export PRJ_DIR := $(CURDIR)
$(eval $(call FORBID_EMPTY_SYMBOL,PRJ_DIR,$(PRJ_DIR),Invalid PRJ_DIR symbol))

#
# Global file/directory symbols
#


export PRJ_DIR_CFG := $(PRJ_DIR)/cfg

export PRJ_DIR_HOST := $(PRJ_DIR)/host
export PRJ_DIR_SRC := $(PRJ_DIR)/src
export PRJ_DIR_IMPORT := $(PRJ_DIR)/import

export PRJ_DIR_STAGE := $(PRJ_DIR)/stage
export PRJ_DIR_STAGE_INC := $(PRJ_DIR_STAGE)/include
export PRJ_DIR_STAGE_LIB := $(PRJ_DIR_STAGE)/lib

export PRJ_DIR_RELEASE := $(PRJ_DIR)/release

export PRJ_DIR_PACK := $(PRJ_DIR)/pack
export PRJ_DIR_PACK_INC := $(PRJ_DIR_PACK)/include
export PRJ_DIR_PACK_BIN := $(PRJ_DIR_PACK)/bin
export PRJ_DIR_PACK_LIB := $(PRJ_DIR_PACK)/lib
export PRJ_DIR_PACK_MOD := $(PRJ_DIR_PACK)/modules
export PRJ_DIR_PACK_ETC := $(PRJ_DIR_PACK)/etc

export PRJ_PATH_AUTOCONF := $(CURDIR)/autoconf.mk

##################################

#
# Default target
#
.PHONY: default
default: all

.PHONY: all
all:: verify_local_exec oldconfig verify_autoconf_output

#
# Check for used executables.
#
.PHONY: verify_local_exec
verify_local_exec:
	@echo "...Verify local host exec"
	@$(call CMD_CHECK_EXEC,gcc)
	@$(call CMD_CHECK_EXEC,tar)
	@$(call CMD_CHECK_EXEC,sed)
	@$(call CMD_CHECK_EXEC,awk)
	@$(call CMD_CHECK_EXEC,gperf)
	@$(call CMD_CHECK_EXEC,bison)
	@$(call CMD_CHECK_EXEC,flex)
	@$(call CMD_CHECK_EXEC,grep)
	@$(call CMD_CHECK_EXEC,install)

.PHONY: verify_autoconf_output
verify_autoconf_output:
	@echo "...Verify autoconf output (.config)"
	@$(call CMD_CHECK_REG_FILE,$(PRJ_PATH_AUTOCONF))

#
# kconfig
#
kconfig-dir:= $(PRJ_DIR_HOST)/kconfig-frontends/.inst/usr/bin/
kconfig-conf := $(kconfig-dir)/kconfig-conf
kconfig-menuconfig := $(kconfig-dir)/kconfig-mconf
kconfig-oldconfig := $(kconfig-conf) --oldaskconfig 
kconfig-silentoldconfig := $(kconfig-conf) --silentoldconfig
kconfig-defconfig := $(kconfig-conf) --defconfig

kconfig-input := Kconfig
kconfig-output := $(PRJ_PATH_AUTOCONF)

.PHONY: prepare
prepare:: host

.PHONY: menuconfig
menuconfig: verify_local_exec prepare
	@echo "...Generating autoconf $(PRJ_PATH_AUTOCONF)"
	@$(call CMD_FORBID_EMPTY_STR,PRJ_PATH_AUTOCONF,$(PRJ_PATH_AUTOCONF),Invalid symbol PRJ_PATH_AUTOCONF)
	$(kconfig-menuconfig) $(kconfig-input)
	

.PHONY: oldconfig
oldconfig: silentoldconfig

.PHONY: silentoldconfig
silentoldconfig: verify_local_exec prepare
	@echo "...Generating autoconf $(PRJ_PATH_AUTOCONF)"
	@$(call CMD_FORBID_EMPTY_STR,PRJ_PATH_AUTOCONF,$(PRJ_PATH_AUTOCONF),Invalid symbol PRJ_PATH_AUTOCONF)
	@echo "...Prepare dir for autoconf output"
	@mkdir -vp $(PRJ_DIR_STAGE_INC)/config
	@$(call CMD_CHECK_DIR,$(PRJ_DIR_STAGE_INC)/config)
	@mkdir -vp $(PRJ_DIR_STAGE_INC)/generated/
	@$(call CMD_CHECK_DIR,$(PRJ_DIR_STAGE_INC)/generated)
	@$(kconfig-silentoldconfig) $(kconfig-input)
	@$(call CMD_CHECK_REG_FILE,$(PRJ_DIR_STAGE_INC)/generated/autoconf.h)

.PHONY: distclean
distclean::
	@-rm -vf $(kconfig-output) $(kconfig-output).old

pack-dir-y :=
pack-dir-y += "$(PRJ_DIR_PACK)"

.PHONY: prepare_pack_dir
prepare_pack_dir:
	@$(foreach d,$(pack-dir-y),$(call CMD_PREP_DIR,$(d)))

prepare:: prepare_pack_dir

.PHONY: clean_pack_dir
clean_pack_dir:
	@if [ ! "$(PRJ_DIR_PACK)" = "/" -a -d "$(PRJ_DIR_PACK)" ]; then rm -rvf "$(PRJ_DIR_PACK)"; fi;

.PHONY: clean_stage_dir
clean_stage_dir:
	@if [ ! "$(PRJ_DIR_STAGE)" = "/" -a -d "$(PRJ_DIR_STAGE)" ]; then rm -rvf "$(PRJ_DIR_STAGE)"; fi

.PHONY: clean
clean:: clean_pack_dir clean_stage_dir

.PHONY: distclean
distclean:: clean_pack_dir clean_stage_dir

#
# subdir_target_tpl: All subdir targets should be the same with this
# 
define subdir_target_tpl
$(1)-dir-y += $(1)

.PHONY: $(1)
$(1): $(1)_all $(1)_install

.PHONY: $(1)_all
$(1)_all:
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),all)

.PHONY: $(1)_clean
$(1)_clean:
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),clean) 

.PHONY: $(1)_install
$(1)_install::
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),install) 

.PHONY: $(1)_distclean
$(1)_distclean::
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),distclean) 

.PHONY: $(1)_mrproper
$(1)_mrproper::
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),mrproper)

.PHONY: $(1)_release
$(1)_release::
	@$$(call CMD_MAKE_ALL_DIR_NOSP,$$($(1)-dir-y),release)

# Run build + install export files into pack.
.PHONY: all
all:: $(1)

.PHONY: clean
clean:: $(1)_clean

.PHONY: distclean
distclean:: $(1)_distclean

.PHONY: mrproper
mrproper:: $(1)_mrproper

.PHONY: install
install:: $(1)_install

.PHONY: release
release:: $(1)_release

endef # subdir_target_tpl

subdir-y :=
subdir-y += $(PRJ_DIR_HOST)
ifeq ($(d),)
subdir-y += $(PRJ_DIR_IMPORT)
subdir-y += $(PRJ_DIR_SRC)
endif

$(foreach d,$(subdir-y),$(eval $(call subdir_target_tpl,$(notdir $(d)))))

ifneq ($(d),)
# Allow development syntax: make d=userspace/busybox all install
.PHONY: all
$(if $(MAKECMDGOALS), $(MAKECMDGOALS), all):: verify_local_exec oldconfig verify_autoconf_output
	$(MAKE) -C "$(d)" $(MAKECMDGOALS)
endif

#
# All general phony targets should be defined here.
#
$(eval $(call DEFINE_GENERAL_TARGET))

#
# release utility: release purge (stand-alone target)
#
.PHONY: release_purge
release_purge:
	@echo "...Purge existed release packages"
	@rm -vf $(PRJ_DIR_RELEASE)/*

#
# release
#
release_verify:
	@echo "...Check package directory $(PRJ_DIR_PACK)"
	@$(call CMD_CHECK_DIR,$(PRJ_DIR_PACK))

package_name_pfx := templatev1

package_ver := v0.0
package_date := $(shell date "+%Y%m%d")
package_name := $(package_name_pfx)_$(package_ver)_$(package_date)
package_path := $(PRJ_DIR_RELEASE)/$(package_name).tgz

.PHONY: release_pack
release_pack:
	@echo "Package name is: $(package_name)"
	@mkdir -vp "$(package_name)" && echo "...Copy files" && cp -rvf "$(PRJ_DIR_PACK)"/* "$(package_name)"
	@echo "...tar release package" && tar -zcvf "$(package_path)" "$(package_name)" # Use some other extension?
	@rm -rf "$(package_name)"
	@-echo "Release package info:" && stat "$(package_path)"
    
.PHONY: release_sync
release_sync:
	@-sync; sync; sync;


release:: release_verify release_pack release_sync

#
# clone
#
clone-dir := $(CURDIR)
clone-date := $(shell date "+%Y%m%d")
clone-tmp := /tmp/$(package_name_pfx)_src_$(clone-date).tgz
.PHONY: clone
clone:
	@echo "...Clone current source codes"
	@$(call CMD_CHECK_DIR,$(clone-dir))
	@echo "...Compress dir '$(clone-dir)'"
	cd $(clone-dir) && tar -zcvf "$(clone-tmp)" ./
	@-stat $(clone-tmp)
	@$(call CMD_CP_NOSP,$(clone-tmp),$(PRJ_DIR_RELEASE))


#
# tags (Need ctags and cscope)
#
# * NOTE: This's convenient for vim geeks. 
#
.PHONY: tags
tags:
	@echo "...Generate file list"
	@find src app -name "*.[ch]" > .ctags_filelist.txt
	@echo '...Parse files by ctags'
	@ctags -I Struct=struct -L .ctags_filelist.txt
	@echo "...Remove temporary file list"
	@rm -f .ctags_filelist.txt
	@echo '...Run cscope'
	@rm -f cscope.* && cscope -Isrc -Iapp -R -b -k -q


#;
