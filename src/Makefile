include Makefile.builtem

EXECUTABLES = korektor korektor_vertical create_error_model create_lm_binary create_morphology tokenize
LIBRARIES = libkorlib

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,.)

UNILIB_O = unilib/unicode unilib/uninorms unilib/utf8 unilib/utf16
KORLIB_O = korlib/CompIncreasingArray korlib/Configuration korlib/DecoderBase korlib/DecoderMultiFactor korlib/LMWrapper korlib/Lexicon korlib/MyCash2 korlib/MyIncreasingArray korlib/MyMappedDoubleArray korlib/MyPackedArray korlib/MyUTF korlib/NGram korlib/SimWordsFinder korlib/Spellchecker korlib/StagePosibility korlib/TextCheckingResult korlib/Token korlib/ValueMapping korlib/ViterbiState korlib/ZipLM korlib/ZipLM_creation korlib/constants korlib/utils $(UNILIB_O)

# library
$(call lib,libkorlib): $(call obj,$(KORLIB_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,korektor): $(call obj,korektor $(KORLIB_O))
$(call exe,korektor_vertical): $(call obj,korektor_vertical $(KORLIB_O))
$(call exe,create_error_model): $(call obj,tools/create_error_model $(KORLIB_O))
$(call exe,create_lm_binary): $(call obj,tools/create_lm_binary $(KORLIB_O))
$(call exe,create_morphology): $(call obj,tools/create_morphology $(KORLIB_O))
$(call exe,tokenize): $(call obj,tools/tokenize $(KORLIB_O))
$(call exe,$(EXECUTABLES)):
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
