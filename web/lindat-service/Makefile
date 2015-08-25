TARGETS = .htaccess about.html api-reference.php demo.php footer.php header.php icon.png info.php korektor.css log.php
TARGETS += weblicht/czech_spellcheck.cmdi weblicht/spellcheck.wadl weblicht/strip_diacritics.cmdi

.PHONY: all
all: $(TARGETS)

../../documentation/manual_about.html:
	$(MAKE) -C $(@D) $(@F)

about.html: ../../documentation/manual_about.html
	sed -n '/^<div class="body"/,/^<\/div/p' $< | sed 's/<div class="body"[^>]*>/<div>/' >$@

install: $(TARGETS)
	scp $(TARGETS) quest:/var/www/korektor/

.PHONY: clean
clean:
	rm -f about.html
