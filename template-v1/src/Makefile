#
# Prerequisites
#
include $(PRJ_MK_SHARED)

#
# Get .o members to obj-y
#
include $(CURDIR)/Makefile.inc
ifeq ($(filter clean distclean,$(MAKECMDGOALS)),)
$(eval $(call FORBID_EMPTY_SYMBOL,obj-y,$(obj-y)))
endif

my-cflags :=
my-cflags += $(my-cflags-y)
my-cflags += $(CONFIG_TC_EXTRA_CFLAGS)
my-cflags += $(CONFIG_TEMPLATE_EXTRA_CFLAGS)

my-ldflags :=
my-ldflags += $(my-ldflags-y)
my-ldflags += $(CONFIG_TC_EXTRA_LDFLAGS)
my-ldflags += $(CONFIG_TEMPLATE_EXTRA_LDFLAGS)

exec := template_exec

.PHONY: default
default: all

.PHONY: all
all:: $(exec)

# Acquire auto-generated dependency targets from .d files.
-include $(obj-y:.o=.d)

$(obj-y):
	@$(call CMD_CC,$@,$(CONFIG_TC_CC) $(my-cflags) -o $@ -c $(patsubst %.o,%.c,$@))
	-@$(CONFIG_TC_CC) -MP -MM $(patsubst %.o,%.c,$@) -MT $@ $(my-cflags) > $(patsubst %.o,%.d,$@)

$(exec): $(obj-y)
	@$(call CMD_CC,$@,$(CONFIG_TC_CC) -o $@ $^ $(my-ldflags)) 

.PHONY: clean
clean::
	-@rm -vf $(obj-y:.o=.o.log)
	-@rm -vf $(obj-y:.o=.d)
	-@rm -vf $(obj-y) $(exec)

.PHONY: install
install::
	@$(call CMD_PREP_DIR,$(PRJ_DIR_PACK_BIN))
	@$(call CMD_CP_NOSP,$(exec),$(PRJ_DIR_PACK_BIN))

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
