.PHONY: all plugin
all: plugin
plugin: addon-sdk-1.17 $(addprefix ../common/,AUTHORS CHANGES LICENSE README jquery-2.1.3.min.js korektor_48.png spellcheck.js) $(wildcard data/* lib/* locale/*) package.json
	@mkdir -p build
	cp -r data/ lib/ locale/ package.json build
	cp -r $(addprefix ../common/,AUTHORS CHANGES LICENSE README) build
	cp -r $(addprefix ../common/,jquery-2.1.3.min.js spellcheck.js korektor_48.png) build/data

xpi: plugin
	addon-sdk-1.17/bin/cfx --pkgdir=build xpi
	unzip -c korektor_spellchecker.xpi install.rdf | sed '/<em:homepageURL>/r install.rdf.add' >build/install.rdf
	zip korektor_spellchecker.xpi --junk-paths build/install.rdf $(addprefix build/,AUTHORS CHANGES LICENSE README)

run: plugin
	addon-sdk-1.17/bin/cfx --pkgdir=build run --binary-args=../test.html

addon-sdk-1.17:
	@[ -d $@ ] || curl https://ftp.mozilla.org/pub/mozilla.org/labs/jetpack/addon-sdk-1.17.tar.gz | tar xz

.PHONY: clean
clean:
	rm -rf build korektor_spellchecker.xpi
