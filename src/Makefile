# This file is part of korektor <http://github.com/ufal/korektor/>.
#
# Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
# of Mathematics and Physics, Charles University in Prague, Czech Republic.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted under 3-clause BSD licence.

include Makefile.builtem

EXECUTABLES = korektor create_error_model create_lm_binary create_morphology tokenizer
LIBRARIES = libkorlib

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,.)

KORLIB_O = decoder/decoder_base decoder/decoder_multi_factor decoder/stage_possibility decoder/viterbi_state
KORLIB_O += error_model/error_model error_model/error_model_basic
KORLIB_O += language_model/lm_wrapper language_model/ngram language_model/zip_lm language_model/zip_lm_creation
KORLIB_O += lexicon/lexicon lexicon/sim_words_finder morphology/morphology
KORLIB_O += persistent_structures/bit_array persistent_structures/comp_increasing_array persistent_structures/increasing_array persistent_structures/mapped_double_array persistent_structures/packed_array persistent_structures/string_array persistent_structures/value_mapping
KORLIB_O += spellchecker/configuration spellchecker/constants spellchecker/spellchecker
KORLIB_O += token/input_format token/output_format token/token token/tokenizer
KORLIB_O += unilib/unicode unilib/utf16 unilib/utf8 unilib/version
KORLIB_O += utils/io utils/options utils/parse utils/utf version/version

# library
$(call lib,libkorlib): $(call obj,$(KORLIB_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,korektor): $(call obj,korektor $(KORLIB_O))
$(call exe,create_error_model): $(call obj,create/create_error_model $(KORLIB_O))
$(call exe,create_lm_binary): $(call obj,create/create_lm_binary $(KORLIB_O))
$(call exe,create_morphology): $(call obj,create/create_morphology $(KORLIB_O))
$(call exe,tokenizer): $(call obj,tokenizer $(KORLIB_O))
$(call exe,$(EXECUTABLES)):
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
