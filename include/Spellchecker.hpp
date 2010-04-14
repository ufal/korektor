/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef SPELLCHECKER_HPP_
#define SPELLCHECKER_HPP_

#include "StdAfx.h"
#include "StagePosibility.hpp"
#include "DecoderBase.hpp"

namespace ngramchecker {

	//class StagePosibility;
	//SP_DEF(StagePosibility);
	
	//class TransitionCostComputation;
	//SP_DEF(TransitionCostComputation);

	class TextCheckingResult;
	SP_DEF(TextCheckingResult);

	class Configuration;
	SP_DEF(Configuration);

	//class DecoderBase;
	//SP_DEF(DecoderBase);


	//Spellchecker class the main interface of the application. It receives spell-checking request and delivers the corrected text.
	//The responsibility of Spellchecker is to prepare input for the decoder (i.e. tokenize the sentence), call decoder and process the decoder output in the desired way. 
	class Spellchecker {

		Configuration* configuration;

		shared_ptr<DecoderBase> decoder;

		map<uint32_t, vector<StagePosibilityP> > MakeSuggestionList(vector<StagePosibilityP> &decoded_pos, StagePosibilitiesType stage_posibilities);	
	
	public:

		//this was relevant for the spell-server front-end
		vector<TextCheckingResultP> GetCheckingResults(const string &sentence);
		
		//this was relevant for the spell-server front-end
		vector<string> GetContextFreeSuggestions(const string &word);

		//this was relevant for the spell-server front-end
		void FindMisspelledWord(const string &text, uint32_t &range_from, uint32_t &range_length);
		
		//this was relevant for the spell-server front-end
		vector<TextCheckingResultP> GetCheckingResultsFirstSentence(const string &text, uint &range_from, uint &range_length);

		//returns autocorrected text
		string CheckText(const string &sentence);

		//marks correction in a way that is useful for evaluation - make distinction between real-word-errors and normal spelling errors
		string DecodeEvaluation(const string &text, uint32_t num_sugg_to_output);
		
		//marks mispelled words and correction using a XML tag
		string command_line_mode(const string &text, uint32_t num_sugg_to_output);

		Spellchecker(Configuration* _configuration);

	};

}

#endif /* SPELLCHECKER_HPP_ */
