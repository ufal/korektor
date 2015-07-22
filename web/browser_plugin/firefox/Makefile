.PHONY: all plugin
all: plugin
plugin: addon-sdk $(addprefix ../common/,AUTHORS CHANGES LICENSE README jquery-2.1.3.min.js korektor_48.png spellcheck.js) lib/main.js package.json
	addon-sdk/bin/cfx xpi

addon-sdk:
	@[ -d $@ ] || git clone --depth=1 https://github.com/mozilla/addon-sdk 

.PHONY: clean
clean:
	rm -rf build
