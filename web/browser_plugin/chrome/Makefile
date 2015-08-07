.PHONY: all plugin
all: plugin
plugin: $(addprefix ../common/,AUTHORS CHANGES LICENSE README jquery-2.1.3.min.js korektor_128.png korektor_48.png spellcheck.js) _locales korektor.js manifest.json
	@mkdir -p build
	cp -r $^ build/

.PHONY: zip
zip: plugin
	cd build && zip -r ../korektor_spellchecker.zip *

.PHONY: run
run: plugin
	chromium ../test.html

.PHONY: clean
clean:
	rm -rf build korektor_spellchecker.zip
