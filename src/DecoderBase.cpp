/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "DecoderBase.hpp"
#include "StagePosibility.hpp"
#include "Token.hpp"
#include "Configuration.hpp"
#include "Morphology.hpp"

namespace ngramchecker {

	void DecoderBase::init_posibilities(const vector<TokenP> &tokens)
	{
		stage_posibilities = StagePosibilitiesType(new vector<vector<StagePosibilityP> >());

		vector<StagePosibilityP> start_sp;
		start_sp.push_back(sentence_start_SP());

		for (uint32_t i = 0; i < viterbi_order - 1; i++)
		{
			stage_posibilities->push_back(start_sp);
		}

		vector<vector<StagePosibilityP> > inner_sp = init_inner_stage_posibilities(tokens);


		string st_pos_string;
		if (this->configuration->diagnostics)
		{
			cout  << "Stage posibilities:" << endl;

			for (auto it = inner_sp.begin(); it != inner_sp.end(); it++)
			{
				for (auto it2 = it->begin(); it2 != it->end(); it2++)
				{
					StagePosibilityNewP st_pos = std::dynamic_pointer_cast<StagePosibilityNew>(*it2);

					FactorList flist = st_pos->GetFactorList();

					st_pos_string.clear();

					double total_em_cost = 0.0;

					for (uint i = 0; i < configuration->NumFactors(); i++)
					{
						if (configuration->FactorIsEnabled(i))
						{
							if (!st_pos_string.empty()) st_pos_string += "|";
							st_pos_string += configuration->morphology->GetFactorString(i, flist.factors[i]);
						}

					}

					cout << st_pos_string << " - " << total_em_cost << endl;
				}

				cout << endl;
			}
		}

		stage_posibilities->insert(stage_posibilities->end(), inner_sp.begin(), inner_sp.end());

		vector<StagePosibilityP> end_sp;
		end_sp.push_back(sentence_end_SP());

		stage_posibilities->push_back(end_sp);

	}


	uint32_t DecoderBase::GetViterbiOrder()
	{
		return viterbi_order;
	}

	vector<StagePosibilityP> DecoderBase::DecodeTokenizedSentence(const vector<TokenP> &tokens)
	{

		//cerr << "decoded init...\n";

		vector<StagePosibilityP> ret_vec;

		if (tokens.size() == 0)
		{
			return ret_vec;
		}

		init_posibilities(tokens);



		Trellis_stage_set new_trellis_stage;
		Trellis_stage last_trellis_stage = Trellis_stage();

		last_trellis_stage.reserve(1000);

		vector<StagePosibilityP> start_state_history;

		for (uint32_t i = 0; i < viterbi_order - 1; i++)
			start_state_history.push_back(sentence_start_SP());

		ViterbiStateP init_vsP = ViterbiStateP(new ViterbiState(start_state_history));
		last_trellis_stage.push_back(init_vsP);

		//cerr << "...Decoder start...\n";


		for (uint32_t i = viterbi_order - 1; i < stage_posibilities->size(); i++)
		{
			vector<StagePosibilityP> &current_stage_posibilities = stage_posibilities->operator [](i);
			new_trellis_stage.clear();

			//cerr << "stage = " << i << ", possibilities = " << current_stage_posibilities.size() << endl;

			double best = 100000000;

			assert(last_trellis_stage.size() > 0);

			for (uint32_t j = 0; j < last_trellis_stage.size(); j++)
			{
				ViterbiStateP &viterbi_stateP = last_trellis_stage[j];



				for (uint32_t k = 0; k < current_stage_posibilities.size(); k++)
				{

					StagePosibilityP stage_pos = current_stage_posibilities[k];

					double cost = viterbi_stateP->distance;
					double emmision_cost = stage_pos->EmmisionProbability();

					cost += emmision_cost;

					//cerr << "distance: " << viterbi_stateP->distance << ", emmision: " << emmision_cost << endl;

					assert(cost < 100000000);

					if (cost > best + MyConstants::prunning_constant)
						continue;

					double transition_cost = ComputeTransitionCost(viterbi_stateP, stage_pos);

					if (configuration->diagnostics)
					{
						cout << "transition: " << viterbi_stateP->ToString() << " --> " << stage_pos->ToString() << " ~ " << transition_cost << endl << endl;
					}


					/*if (transition_cost < 0)
					{
						cerr << "!!! transition_cost < 0 !!! == " << transition_cost << ", (might happen, because of prob. values quantizations)" <<endl;
						//exit(1);
					}*/

					cost += transition_cost;

					if (cost > best + MyConstants::prunning_constant)
						continue;

					if (cost < best)
					{
						//cerr << "old best = " << best << ", new best = " << cost << "(emm = " << emmision_cost << ", trans = " << transition_cost << ")" << endl;
						best = cost;
					}

					//LMStateKeyP new_lm_state_keyP = LMStateKeyP(new LMStateKey(viterbi_stateP->lmState, wf_cp.word_factors));
					//GlobalVars::lmServer->Reduce(new_lm_state_keyP);

					ViterbiStateP new_viterbi_stateP = ViterbiStateP(new ViterbiState(viterbi_stateP, stage_pos, cost));

					//TODO: Wouldn't it be actually better to store the new states in a vector and sort and make unique afterwards?
					Trellis_stage_set::iterator it = new_trellis_stage.find(new_viterbi_stateP);

					if (it == new_trellis_stage.end())
					{
						new_trellis_stage.insert(new_viterbi_stateP);
					}
					else
					{
						ViterbiStateP stored_state = *it;
						if (stored_state->distance > new_viterbi_stateP->distance)
						{
							stored_state->distance = new_viterbi_stateP->distance;
							stored_state->ancestor = new_viterbi_stateP->ancestor;
						}

					}

				}

			}

			last_trellis_stage.clear();


			for (Trellis_stage_set::iterator it = new_trellis_stage.begin(); it != new_trellis_stage.end(); it++)
			{
				ViterbiStateP state = *it;
				if (state->distance < best + MyConstants::prunning_constant)
				{
					last_trellis_stage.push_back(state);
				}
			}

			std::sort(last_trellis_stage.begin(), last_trellis_stage.end(), ViterbiStateP_compare_distance());
			//cerr << "last_trellis_stage->size() = " << last_trellis_stage->size() << endl;
		}

		assert(last_trellis_stage.size() > 0);

		ViterbiStateP state = last_trellis_stage.operator [](0);

		ret_vec.push_back(state->GetYoungestHistory());

		while (state->ancestor)
		{
			state = state->ancestor;
			ret_vec.push_back(state->GetYoungestHistory());
		}

		if (viterbi_order > 1)
		{
			for (uint32_t i = 0; i < viterbi_order - 2; i++)
				ret_vec.push_back(sentence_start_SP());
		}

		assert(ret_vec.size() == stage_posibilities->size());

		std::reverse(ret_vec.begin(), ret_vec.end());
		return ret_vec;
	}

	void DecoderBase::DecodeTokenizedSentence_ReturnStagePosibilities(const vector<TokenP> &tokens, vector<StagePosibilityP> &decoded_sequence, StagePosibilitiesType &_stage_posibilities)
	{
		decoded_sequence = DecodeTokenizedSentence(tokens);
		_stage_posibilities = stage_posibilities;
	}

	DecoderBase::DecoderBase(Configuration* _configuration)
	{
		configuration = _configuration;
		viterbi_order = _configuration->viterbi_order;
	}

}
