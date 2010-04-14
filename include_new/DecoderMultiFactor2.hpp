/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef DECODER2_HPP_
#define DECODER2_HPP_

#include "StdAfx.h"
#include "ViterbiState.hpp"
#include "Token.hpp"

#include "StagePosibility2.hpp"
#include "ViterbiState2.hpp"
#include "Configuration.hpp"
#include "Morphology.hpp"
#include "NGram.hpp"
#include "LinearHash.hpp"
#include "LMWrapper.hpp"
#include "ZipLM.hpp"
#include "ErrorModel.hpp"
#include "SimWordsFinder.hpp"

namespace ngramchecker {


typedef shared_ptr<vector<vector<StagePosibilityP> > > StagePosibilitiesType;

template<int FACTORS, int ORDER>
class Decoder2
{

private:

	LinearHash<uint, ViterbiState2<FACTORS, ORDER - 1>> trellis_stage_next;

	vector<pair<ViterbiState2<FACTORS, ORDER - 1>*, uint>> trellis_old;


	StagePosibility2<FACTORS> sentence_start_sp;
	StagePosibility2<FACTORS> sentence_end_sp;
	StagePosibility2<FACTORS> unknown_sp;

	ViterbiState2<FACTORS, ORDER - 1> sentence_start_viterbi_state;

	LMWrapper* lmWrappers[FACTORS];
	float factorWeights[FACTORS];
	uint factorOrders[FACTORS];

	SimWordsFinderP simWordsFinder;
	ErrorModelP errorModel;
	MorphologyP morphology;
	Configuration* configuration;

	NGram ngram_search_key;
	NGram ngram_val;

	vector<StagePosibility2<FACTORS>> st_pos; //all stage posibilities for all words
	vector<uint> st_pos_first_ind; //indices of first stage posibility for each level

	vector<uint> factor_indices;

	enum { NOT_A_FACTOR = UINT32_MAX };

	struct st_pos_comparer : public less<StagePosibility2<FACTORS>>
	{
		bool operator()(const StagePosibility2<FACTORS> &val1, const StagePosibility2<FACTORS> &val2)
		{
			return val1.emission_prob < val2.emission_prob;
		}
	};

	void init_stage_posibilities(const vector<TokenP> &tokens)
	{
		st_pos.clear();
		st_pos_first_ind.clear();

		for (uint i = 0; i < ORDER - 1; i++)
		{
			st_pos_first_ind.push_back(st_pos.size());
			st_pos(sentence_start_sp);
		}

		for (uint i = 0; i < tokens.size(); i++)
		{
			st_pos_first_ind.push_back(st_pos.size());

			u16string& u_word = tokens[i]->str_u16;

			vector<StagePosibilityP> vec_stage_pos;

			//auto pair = make_pair(tokens[i]->str_utf8, tokens[i]->sentence_start);
				
			//if (configuration->st_pos_multifactor_cash->IsCashed(pair))
			//{
			//	vec_stage_pos = configuration->st_pos_multifactor_cash->GetCashedValue(pair);
			//}
			//else
			//{
			Similar_Words_Map msw = simWordsFinder->Find(tokens[i]);

			for (Similar_Words_Map::iterator it = msw.begin(); it != msw.end(); it++)
			{
				uint32_t sim_word_id = it->first;
				u16stringP sim_word_str = it->second.first;
				bool is_original = errorModel->StringsAreIdentical(u_word, *sim_word_str);
				double err_model_cost = it->second.second;

				StagePosibility2<FACTORS> stage_pos;

				vector<FactorList> f_lists = morphology->GetMorphology(sim_word_id, configuration);

				for (auto it2 = f_lists.begin(); it2 != f_lists.end(); it2++)
				{
					stage_pos.emission_prob = err_model_cost;

					for (uint i = 0; i < factor_indices.size(); i++)
					{
						stage_pos.emission_prob += it2->emission_costs[factor_indices[i]] * factorWeights[i];			
						stage_pos.factors[i] = it2->factors[factor_indices[i]];
					}

					stage_pos.is_original = is_original;
					stage_pos.word = *sim_word_str;

					st_pos.push_back(stage_pos);
				}

			}

			if (vec_stage_pos.empty())
				st_pos.push_back(unknown_sp);

			std::sort(st_pos.begin() + st_pos_first_ind.back(), st_pos.end(), st_pos_comparer());


			//std::sort(vec_stage_pos.begin(), vec_stage_pos.end(), StagePosibility_sort_cost());
			//configuration->st_pos_multifactor_cash->StoreValueForKey(pair, vec_stage_pos);
			//}


			//ret.push_back(vec_stage_pos);


		}

		st_pos_first_ind.push_back(st_pos.size());
		st_pos.push_back(sentence_end_sp);
		st_pos_first_ind.push_back(st_pos.size());
	}

	bool MakeTransition(const ViterbiState2<FACTORS, ORDER - 1> &old_state, const StagePosibility2<FACTORS> &next, ViterbiState2<FACTORS, ORDER - 1> &new_state, float prunning_distance)
	{
		LM_tuple lm_tuple;

		float new_dist = old_state.distance;
		new_dist += next.emission_prob;

		if (new_dist > prunning_distance)
			return false;		

		for (uint i = 0; i < FACTORS; i++)
		{
			float last_prob = 10;// NOT_IN_LM_COST;
			bool found = false;
			
			//getting lm_tuple for unigram
			if (lmWrappers[i]->getFirstLevelTuple(next.factors[i], lm_tuple))
			{
				last_prob = lm_tuple.prob;
				new_state.offset[i][0] = lm_tuple.nlevel_offset;
				new_state.num_entries[i][0] = lm_tuple.nlevel_entries;
				new_state.bow[i][0] = lm_tuple.bow;

				uint j = 0;
				while (j < ORDER - 1 && old_state.offset[i][j] != NOT_A_FACTOR && lmWrappers[i]->GetTuple(j + 1, next.factors[i], old_state.offset[i][j], old_state.num_entries[i][j], lm_tuple))
				{
					last_prob = lm_tuple.prob;

					if (j < ORDER - 2)
					{
						new_state.offset[i][j + 1] = lm_tuple.nlevel_offset;
						new_state.num_entries[i][j + 1] = lm_tuple.nlevel_entries;
						new_state.bow[i][j + 1] = lm_tuple.bow;
					}

					j++;
				}

				float distance_inc = last_prob;

				while (j < ORDER - 1)
				{
					distance_inc += old_state.bow[i][j];

					if (j < ORDER - 2)
					{
						new_state.offset[i][j + 1] = NOT_A_FACTOR;
						new_state.num_entries[i][j + 1] = 0;
						new_state.bow = 0.0f;
					}
				}

				new_dist += distance_inc * factorWeights[i];

				if (new_dist > prunning_distance)
					return false;
			}
			else
			{
				float distance_inc = last_prob;

				for (uint j = 0; j < ORDER - 1; j++)
					distance_inc += old_state.bow[i][j];

				new_dist += distance_inc * factorWeights[i];

				if (new_dist > prunning_distance)
					return false;
			}

		}

		new_state.distance = new_dist;
		new_state.ancestor = old_state;
		new_state.stage_posibility = next;

		new_state.ComputeHash();

		return true;
	}

public:

	Decoder2(Configuration* _configuration): trellis_stage_next(16, 0.5f), ngram_search_key(ORDER), ngram_val(ORDER)
	{
		configuration = _configuration;
		for (uint i = 0; i < _configuration->NumFactors(); i++)
		{
			if (_configuration->FactorIsEnabled(i))
				factor_indices.push_back(i);
		}

		assert(factor_indices.size() == FACTORS);

		vector<FactorList> mnodes_start = _configuration->morphology->GetMorphology(MyConstants::sentence_start_id, _configuration);
		vector<FactorList> mnodes_end = _configuration->morphology->GetMorphology(MyConstants::sentence_end_id, _configuration);
		vector<FactorList> mnodes_unknown = _configuration->morphology->GetMorphology(MyConstants::unknown_word_id, _configuration);

		sentence_start_sp.emission_prob = 0;
		sentence_end_sp.emission_prob = 0;

		sentence_start_sp.word = MyUtils::utf8_to_utf16("<s>");
		sentence_end_sp.word = MyUtils::utf8_to_utf16("</s>");

		unknown_sp.emission_prob = _configuration->errorModel->UnknownWordCost();

		for (uint i = 0; i < factor_indices.size(); i++)
		{			
			sentence_start_sp.factors[i] = mnodes_start[0].factors[factor_indices[i]];
			sentence_end_sp.factors[i] = mnodes_end[0].factors[factor_indices[i]];
			unknown_sp.factors[i] = mnodes_unknown[0].factors[factor_indices[i]];
		}

		for (uint i = 0; i < factor_indices.size(); i++)
		{
			lmWrappers[i] = _configuration->GetFactorLM(factor_indices[i]);
			factorWeights[i] = _configuration->GetFactorWeight(factor_indices[i]);
			factorOrders[i] = _configuration->GetFactorOrder(factor_indices[i]);
		}

		simWordsFinder = _configuration->simWordsFinder;
		errorModel = _configuration->errorModel;
		morphology = _configuration->morphology;
		//lmWrappers[i]->getFirstLevelTuple(next.factors[i], lm_tuple);
		//lmWrappers[i]->GetTuple(j + 1, next.factors[i], old_state.offset[i][j], old_state.num_entries[i][j], lm_tuple);
		
		LM_tuple lm_tuple;
		for (uint i = 0; i < FACTORS; i++)
		{
			lmWrappers[i]->getFirstLevelTuple(MyConstants::sentence_start_id, lm_tuple);

			sentence_start_viterbi_state.bow[i][0] = lm_tuple.bow;
			sentence_start_viterbi_state.offset[i][0] = lm_tuple.nlevel_offset;
			sentence_start_viterbi_state.num_entries[i][0] = lm_tuple.nlevel_entries;
			sentence_start_viterbi_state.stage_posibility = &sentence_start_sp;
			sentence_start_viterbi_state.ancestor = nullptr;
			sentence_start_viterbi_state.distance = 0.0f;

			for (uint j = 1; j < ORDER - 1; j++)
			{
				lmWrappers[i]->GetTuple(j, MyConstants::sentence_start_id, lm_tuple.nlevel_offset, lm_tuple.nlevel_entries, lm_tuple);
				sentence_start_viterbi_state.bow[i][j] = lm_tuple.bow;
				sentence_start_viterbi_state.offset[i][j] = lm_tuple.nlevel_offset;
				sentence_start_viterbi_state.num_entries[i][j] = lm_tuple.nlevel_entries;
			}

		}

		sentence_start_viterbi_state.ComputeHash();

		sentence_start_sp.is_original = true;
		sentence_end_sp.is_original = true;
		unknown_sp.is_original = false;

		//TODO: initialization of the u16string forms!

	}

	float ComputeTransitionCostSPSequence(vector<StagePosibility2<FACTORS>> &sp_vec, uint32_t start_index, uint32_t end_index)
	{
		float ret_cost = 0;
		
		for (int h = 0; h < FACTORS; h++)
		{
			ngram_search_key.order = factorOrders[h];

			for (uint i = 0; i < ORDER; i++)
				ngram_search_key.word_ids[i] = sp_vec[i].factors[h];

			ret_cost += factorWeights[h] * lmWrappers[h]->GetProb(ngram_search_key, ngram_val);
		}

		return ret_cost;

	}

	vector<StagePosibility2<FACTORS>>  DecodeTokenizedSentence(const vector<TokenP> &tokens)
	{
		//cerr << "decoded init...\n";

		vector<StagePosibility2<FACTORS>> ret_vec;

		if (tokens.size() == 0)
		{
			return ret_vec;
		}

		trellis_old.clear();
		trellis_old.push_back(make_pair(new ViterbiState2<FACTORS, ORDER - 1>[1], 1));
		trellis_old[0].first[0] = sentence_start_viterbi_state;

		init_stage_posibilities();

		ViterbiState2<FACTORS, ORDER - 1> new_state;
		for (uint i = ORDER - 1; i < st_pos_first_ind.size() - 1; i++)
		{
			ViterbiState2<FACTORS, ORDER - 1>* last_trellis_stage = trellis_old.back().first;
			uint current_stage_num_states = trellis_old.back().second;


			//cerr << "stage = " << i << ", possibilities = " << current_stage_posibilities.size() << endl;

			float best = 100000000.0f;

			assert(last_trellis_stage.size() > 0);

			
			for (uint j = 0; j < current_stage_num_states; j++)
			{
				ViterbiState2<FACTORS, ORDER - 1> &viterbi_state = last_trellis_stage[j];
				
				uint current_stage_first_index = st_pos_first_ind[i];
				uint current_stage_last_index = st_pos_first_ind[i + 1] - 1;

				for (uint32_t k = current_stage_first_index; k <= current_stage_last_index; k++)
				{
					StagePosibility2<FACTORS> &stage_pos = st_pos[k];

					if (MakeTransition(viterbi_state, stage_pos, new_state, best + MyConstants::prunning_constant))
					{
						ViterbiState2<FACTORS, ORDER - 1> *value_present;
						trellis_stage_next.Insert(new_state.hash_code, new_state, value_present);

						if (value_present != nullptr)
						{
							if ((float)(new_state.distance) < (float)(value_present->distance))
								value_present->distance = new_state.distance;
						}

						if ((float)(new_state.distance) < best)
							best = new_state.distance;
					}

				}

			}

			ViterbiState2<FACTORS, ORDER - 1> *trellis_stage_values;
			uint trellis_stage_num_values;

			trellis_stage_next.GetAllValues(trellis_stage_values, trellis_stage_num_values);
			trellis_stage_next.Reset();

			uint next_slot = 0;
			for (uint i = 0; i < trellis_stage_num_values; i++)
			{
				if ((float)(trellis_stage_values[i].distance) < (float)(best + MyConstants::prunning_constant))
				{
					trellis_stage_values[next_slot] = trellis_stage_values[i];
					next_slot++;
				}
			}

			std::sort(trellis_stage_values, trellis_stage_values + next_slot);
			trellis_old.push_back(make_pair(trellis_stage_values, next_slot));
		}


		ViterbiState2<FACTORS, ORDER - 1> *state = &(trellis_old.back().first[0]);
		state = state->ancestor;

		ret_vec.push_back(*(state->stage_posibility));

		while (state->ancestor)
		{
			state = state->ancestor;
			ret_vec.push_back(*(state->stage_posibility));
		}

		if (ORDER > 1)
		{
			for (uint i = 0; i < ORDER - 2; i++)
				ret_vec.push_back(sentence_start_sp);
		}

		std::reverse(ret_vec.begin(), ret_vec.end());

		for (uint i = 0; i < trellis_old.size(); i++)
			delete[] trellis_old[i].first;

		return ret_vec;
	}

	void DecodeTokenizedSentence_ReturnStagePosibilities(const vector<TokenP> &tokens, vector<StagePosibilityP> &decoded_sequence, vector<StagePosibility2<FACTORS>> &_stage_posibilities, vector<uint> &_stage_posibilities_first_ind)
	{
		decoded_sequence = DecodeTokenizedSentence(tokens);
		_stage_posibilities = st_pos;
		_stage_posibilities_first_ind = st_pos_first_ind;
	}

};

}

#endif
