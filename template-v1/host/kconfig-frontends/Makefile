include $(PRJ_MK_SHARED)

INST_DIR := .inst

.PHONY: all clean distclean 
all:: 
	@$(MAKE) -C kconfig-frontends MY_PREFIX=CONFIG_
	@$(MAKE) -C kconfig-frontends install MY_OUTPUT=$(CURDIR)/$(INST_DIR)
	
.PHONY: mrproper
mrproper::
	@$(MAKE) -C kconfig-frontends distclean
	@rm -rvf $(INST_DIR)

$(eval $(call DEFINE_GENERAL_TARGET))
#;