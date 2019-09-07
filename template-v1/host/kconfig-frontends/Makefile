
INST_DIR := .inst

.PHONY: all clean distclean 
all:: 
	@$(MAKE) -C kconfig-frontends
	@$(MAKE) -C kconfig-frontends install MY_OUTPUT=$(CURDIR)/$(INST_DIR) MY_PREFIX=CONFIG_

distclean::
	@$(MAKE) -C kconfig-frontends distclean
	@rm -rvf $(INST_DIR)

clean::
	@$(MAKE) -C kconfig-frontends clean

#
# general
#
.PHONY: all
all::

.PHONY: clean
clean::

.PHONY: distclean
distclean::

.PHONY: install
install::

.PHONY: release
relase::

#;
