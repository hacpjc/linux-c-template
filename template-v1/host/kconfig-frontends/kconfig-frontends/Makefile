#
# Automatically build the latest package.
#

#
# Command (Use default CC): make MY_OUTPUT="/tmp" MY_PREFIX="CONFIG_"
#

MY_OUTPUT ?= "$(CURDIR)/.output"
MY_PREFIX ?= "CONFIG_"

pkg := $(shell ls import/*.tar.xz | sort -r | head -n 1)
pkg-pfx := $(notdir $(patsubst %.tar.xz,%,$(pkg)))

stage-dir := "$(CURDIR)/.stage-$(pkg-pfx)"

stamp := ".stamp-$(pkg-pfx)"

tar-cmd := tar -Jxvf

ifeq ($(pkg),)
$(error Cannot find any pkg at $(CURDIR)/import)
endif

.PHONY: default
default: all

.NOTPARALLEL: all
.PHONY: all
all: msg verify_depend extract_pkg build_pkg

.PHONY: msg
msg:
	@echo "...pkg: $(pkg)"
	@echo "...pkg-pfx: $(pkg-pfx)"
	@echo "...stage-dir: $(stage-dir)"
	@echo "...MY_OUTPUT: $(MY_OUTPUT)"
	@echo "...MY_PREFIX: $(MY_PREFIX)"

.PHONY: verify_depend
verify_depend:
	@which bison
	@which yacc

build_pkg:
	@$(MAKE) -C $(stage-dir)/$(pkg-pfx)

extract_pkg: $(stamp)

opts :=
opts += --disable-wall --disable-shared
opts += --disable-nconf --disable-qconf --disable-gconf
opts += --enable-config-prefix=$(MY_PREFIX)
opts += --disable-L10n

$(stamp):
	@mkdir -vp $(stage-dir)
	$(tar-cmd) $(pkg) -C $(stage-dir)
	@test -d $(stage-dir)/$(pkg-pfx)
	@cd $(stage-dir)/$(pkg-pfx) && ./configure --prefix=/usr $(opts)
	@touch $(stamp)

.PHONY: clean
clean:
	-rm -rf $(stage-dir) $(stamp) .output

.PHONY: distclean
distclean: clean

.PHONY: install
install:
	@test -d $(stage-dir)/$(pkg-pfx)
	@DESTDIR=$(MY_OUTPUT) $(MAKE) -C $(stage-dir)/$(pkg-pfx) install

#;
