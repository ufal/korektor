#pragma once

#include "StdAfx.h"
#include "StagePosibility2.hpp"
#include "DecoderMultiFactor2.hpp"
#include "Configuration.hpp"

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

	template<int FACTORS, int ORDER>
	class Spellchecker2 {

		Configuration* configuration;

		Decoder2<FACTORS, ORDER> decoder;

		map<uint32_t, vector<StagePosibility2<FACTORS>>> MakeSuggestionList(vector<StagePosibility2<FACTORS>> &decoded_pos, const vector<StagePosibility2<FACTORS>> &_stage_posibilities, const vector<uint> &_stage_pos_first_ind);	
	public:

		vector<TextCheckingResultP> GetCheckingResults(const string &sentence);

		string CheckText(const string &sentence);

		string DecodeEvaluation(const string &text, uint32_t num_sugg_to_output);
		
		string command_line_mode(const string &text, uint32_t num_sugg_to_output);

		Spellchecker2(Configuration* _configuration);

	};

}