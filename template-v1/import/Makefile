
include $(CURDIR)/Makefile.inc

subdir := $(subdir-y)

.PHONY: all 
all:: 
	@$(foreach d,$(subdir),$(MAKE) -C $(d) $@ && $(MAKE) -C $(d) install || ext 255;)

.PHONY: clean distclean
clean distclean::
	@$(foreach d,$(subdir),$(MAKE) -C $(d) $@ || ext 255;)

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
