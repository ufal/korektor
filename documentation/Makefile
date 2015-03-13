include ../src/Makefile.builtem
include t2t_docsys/Makefile.include

.PHONY: all doc web install
all: doc
doc: manual.txt manual.html manual_install.txt readme.txt readme_md.md
web:
install: doc
	$(call cp,manual.txt,../MANUAL)
	$(call cp,manual.html,../MANUAL.html)
	$(call cp,manual_install.txt,../INSTALL)
	$(call cp,readme.txt,../README)
	$(call cp,readme_md.md,../README.md)

%.html: %.t2t $(wildcard *.t2t)
	$(call t2t_docsys_html,$@,$<,manual $(if $(findstring _,$*),striplevel))

web%html: manual%t2t $(wildcard *.t2t)
	$(call t2t_docsys_html,$@,$<,web html_addlevel $(if $(findstring _,$*),striplevel))

%.txt: %.t2t $(wildcard *.t2t)
	$(call t2t_docsys_txt,$@,$<,manual $(if $(findstring _,$*),striplevel))

%.md: %.t2t $(wildcard *.t2t)
	$(call t2t_docsys_md,$@,$<,manual $(if $(findstring _,$*),striplevel))

%.pdf: %.t2t $(wildcard *.t2t)
	$(call t2t_docsys_pdf,$@,$<,manual $(if $(findstring _,$*),striplevel))

.PHONY: clean
clean:
	@$(call rm,$(T2T_DOCSYS_OUTPUT_WILDCARDS))
