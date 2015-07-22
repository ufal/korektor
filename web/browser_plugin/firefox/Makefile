.PHONY: all plugin
all: plugin
plugin: addon-sdk-1.17 $(addprefix ../common/,AUTHORS CHANGES LICENSE README jquery-2.1.3.min.js korektor_48.png spellcheck.js) lib/main.js package.json
	addon-sdk-1.17/bin/cfx xpi

run: plugin
	addon-sdk-1.17/bin/cfx run

addon-sdk-1.17:
	@[ -d $@ ] || curl https://ftp.mozilla.org/pub/mozilla.org/labs/jetpack/addon-sdk-1.17.tar.gz | tar xz

.PHONY: clean
clean:
	rm -rf build
