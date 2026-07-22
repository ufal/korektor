.PHONY: all plugin
all: plugin
plugin: AUTHORS CHANGES LICENSE README korektor_128.png korektor_48.png spellcheck.js _locales korektor.js manifest.json
	@mkdir -p build
	cp -r $^ build/

.PHONY: zip
zip: plugin
	cd build && zip -r ../korektor_spellchecker.zip *

.PHONY: run-chromium
run-chromium: plugin
	chromium --user-data-dir=/tmp/chromium-korektor --load-extension=./build ./test.html

.PHONY: run-firefox
run-firefox: plugin
	npx web-ext run --url ./test.html

.PHONY: lint
lint: plugin
	npx web-ext lint -i test.html build/*

.PHONY: clean
clean:
	rm -rf build korektor_spellchecker.zip
