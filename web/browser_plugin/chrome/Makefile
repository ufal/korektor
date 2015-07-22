.PHONY: all plugin
all: plugin
plugin: $(addprefix ../common/,AUTHORS CHANGES LICENSE README jquery-2.1.3.min.js korektor_128.png korektor_48.png spellcheck.js) _locales korektor.js manifest.json
	@mkdir -p build
	cp -r $^ build/

.PHONY: clean
clean:
	rm -rf build
