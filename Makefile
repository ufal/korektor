include Makefile.builtem

EXECUTABLES = korektor korektor_vertical create_error_model create_lm_binary create_morphology tokenize
LIBRARIES = libkorlib

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,include include/unilib)

UNILIB_O = src/unilib/unicode src/unilib/uninorms src/unilib/utf8 src/unilib/utf16
KORLIB_O = src/CompIncreasingArray src/Configuration src/DecoderBase src/DecoderMultiFactor src/LMWrapper src/Lexicon src/MyCash2 src/MyIncreasingArray src/MyMappedDoubleArray src/MyPackedArray src/MyUTF src/NGram src/SimWordsFinder src/Spellchecker src/StagePosibility src/TextCheckingResult src/Token src/ValueMapping src/ViterbiState src/ZipLM src/ZipLM_creation src/constants src/utils $(UNILIB_O)

# library
$(call lib,libkorlib): $(call obj,$(KORLIB_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,korektor): $(call obj,tools/korektor $(KORLIB_O))
$(call exe,korektor_vertical): $(call obj,tools/korektor_vertical $(KORLIB_O))
$(call exe,create_error_model): C_FLAGS+=$(call include_dir,include/create_error_model)
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
