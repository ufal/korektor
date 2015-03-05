MODELS=morphology.bin lm_form.bin lm_lemma.bin lm_tag.bin error.bin

.PHONY: all
all: test.txt.corrected
	diff -q $< $<.gold
	@echo All OK

test.txt.corrected: korektor $(MODELS)
	../../src/korektor model.conf <test.txt >$@

.PHONY: korektor
korektor:
	$(MAKE) -C ../../src

morphology.bin: korektor morphology.txt morphology_counts.txt
	../../src/create_morphology morphology.txt morphology_counts.txt $@ $@.vocab $@.test

lm_%.bin: korektor lm_%.lm morphology.bin
	../../src/create_lm_binary lm_$*.lm morphology.bin morphology.bin.vocab $* 3 $@

error.bin: korektor error.txt
	../../src/create_error_model -binarize error.txt $@

.PHONY: clean
clean:
	rm -rf test.txt.corrected $(MODELS:%=%*)
