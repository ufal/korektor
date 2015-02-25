# This file is part of korektor <http://github.com/ufal/korektor/>.
#
# Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
# of Mathematics and Physics, Charles University in Prague, Czech Republic.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted under 3-clause BSD licence.

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
KORLIB_O = korlib/comp_increasing_array korlib/configuration korlib/decoder_base korlib/decoder_multi_factor korlib/lm_wrapper korlib/lexicon korlib/my_increasing_array korlib/my_mapped_double_array korlib/my_packed_array korlib/my_utf korlib/ngram korlib/sim_words_finder korlib/spellchecker korlib/stage_posibility korlib/text_checking_result korlib/token korlib/value_mapping korlib/viterbi_state korlib/zip_lm korlib/zip_lm_creation korlib/constants korlib/utils $(UNILIB_O)

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
