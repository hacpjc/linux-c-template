include $(PRJ_MK_SHARED)

include $(CURDIR)/Makefile.inc
subdir := $(subdir-y)

.PHONY: default
default:
	@$(foreach d,$(subdir),$(MAKE) -C $(d) || exit 255;)

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS):: 
	@$(foreach d,$(subdir),$(MAKE) -C $(d) $@ || exit 255;)

$(eval $(call DEFINE_GENERAL_TARGET))
#;