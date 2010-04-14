/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "Spellchecker2.hpp"
#include "TextCheckingResult.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "DecoderMultiFactor2.hpp"
#include "Configuration.hpp"
#include "UStringReplacing.hpp"

namespace ngramchecker {

template<int FACTORS>
struct Pair_StagePosibility2_float_comparer: public less<pair<StagePosibility2<FACTORS>, float>>
{
	bool operator()(const pair<StagePosibility2<FACTORS>, float> &val1, const pair<StagePosibility2<FACTORS>, float> &val2)
	{
		if (val1.second < val2.second)
			return true;
		else
			return false;
	}
};

template<int FACTORS, int ORDER>
	map<uint32_t, vector<StagePosibility2<FACTORS>>> Spellchecker2<FACTORS, ORDER>::MakeSuggestionList(vector<StagePosibility2<FACTORS>> &decoded_pos, const vector<StagePosibility2<FACTORS>> &_stage_posibilities, const vector<uint> &_stage_pos_first_ind)
	{

		map<uint32_t, vector<StagePosibility2<FACTORS>>> suggestions;


		for (uint32_t i = ORDER - 1; i < decoded_pos.size() - 1; i++)
		{
			//cerr << "i = " << i << endl;

			if (decoded_pos[i].is_original)
			{
				if (decoded_pos[i].IsUnknown())
				{
					suggestions[i] = vector<StagePosibility2<FACTORS>>();
				}

				continue;
			}

			vector<pair<StagePosibility2<FACTORS>, float> > cost_vector;

			suggestions[i] = vector<StagePosibility2<FACTORS>>();

			StagePosibility2<FACTORS> decoded_pos_at_i_backup = decoded_pos[i];

			for (uint32_t j = _stage_pos_first_ind[i]; j <= _stage_pos_first_ind[i + 1] - 1; j++)
			{
				//cerr << "j = " << j << endl;
				StagePosibility2<FACTORS> sp = _stage_posibilities[j];
				float cost = sp.emission_prob;

				decoded_pos[i] = sp;

				for (uint32_t k = 0; k < ORDER; k++)
				{
					if (i + k >= decoded_pos.size() )
						break;

					cost += decoder.ComputeTransitionCostSPSequence(decoded_pos, i - ORDER + 1 + k, i + k);
				}

				cost_vector.push_back(make_pair(sp, cost));

			}

			decoded_pos[i] = decoded_pos_at_i_backup;
			
			std::sort(cost_vector.begin(), cost_vector.end(), Pair_StagePosibility2_float_comparer<FACTORS>());
			vector<pair<StagePosibility2<FACTORS>, float> > new_cost_vector;
			set<StagePosibility2<FACTORS>, StagePosibility2_Form_comparer> word_ids_in_list;

			for (uint32_t j = 0; j < cost_vector.size(); j++)
			{
				if (cost_vector[j].first.IsOriginal())
					break;

				if (word_ids_in_list.find(cost_vector[j].first) == word_ids_in_list.end())
				{
					word_ids_in_list.insert(cost_vector[j].first);
					new_cost_vector.push_back(cost_vector[j]);
				}
			}


			for (vector<pair<StagePosibility2<FACTORS>, float> >::iterator it = new_cost_vector.begin(); it != new_cost_vector.end(); it++)
			{
				suggestions[i].push_back(it->first);
			}


		}

		//cerr << "... make suggestion finished!" << endl;
		return suggestions;

	}

	template<int FACTORS, int ORDER>
	vector<TextCheckingResultP> Spellchecker2<FACTORS, ORDER>::GetCheckingResults(const string &text)
	{
		u16string u_text = MyUtils::utf8_to_utf16(text);
		vector<TextCheckingResultP> results;

		vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);


		for (auto it = tokens.begin(); it != tokens.end(); it++)
		{
			vector<StagePosibility2<FACTORS>> spv;
			vector<StagePosibility2<FACTORS>> stage_posibs;
			vector<uint> stage_posibs_first_ind;
			decoder.DecodeTokenizedSentence_ReturnStagePosibilities(*it, spv, stage_posibs, stage_posibs_first_ind);

			map<uint32_t, vector<StagePosibility2<FACTORS>> sugg = MakeSuggestionList(spv, stage_posibs, stage_posibs_first_ind);

			for (map<uint32_t, vector<StagePosibility2<FACTORS>> >::iterator it2 = sugg.begin(); it2 != sugg.end(); it2++)
			{
				//assert(it2->first >= decoder_order - 1);
				uint32_t tok_index = it2->first - (ORDER - 1);
				vector<StagePosibility2<FACTORS>> &stage_pos = it2->second;

                TokenP curr_token = (*it)[tok_index];
                vector<string> suggestions;
	
                for (uint32_t i = 0; i < stage_pos.size(); i++)
                {
					suggestions.push_back(MyUtils::utf16_to_utf8(stage_pos[i].word));
                }
	
                if (suggestions.size() > 0 &&
                    MyUtils::CaseInsensitiveIsEqual(stage_pos[0].word, curr_token->str_u16) == false)
                {
                    string sugg = suggestions[0];
                    suggestions.clear();
                    suggestions.push_back(sugg);
                    results.push_back(TextCheckingResultP(new GrammarCheckingResult(curr_token->first, curr_token->length, suggestions, "Error in capitalization? Did you mean " + suggestions[0] + "?", curr_token->str_utf8)));
                }
                else if (curr_token->ID == MyConstants::name_id ||
                    curr_token->ID == MyConstants::unknown_word_id)	 
				{
					results.push_back(TextCheckingResultP(new SpellingCheckingResult(curr_token->first, curr_token->length, suggestions, curr_token->str_utf8)));
				}
				else if (suggestions.size() > 0) //It's not intuitive that size can be zero! However it's possible with agressive prunning during the decoding!
				{
					results.push_back(TextCheckingResultP(new GrammarCheckingResult(curr_token->first, curr_token->length, suggestions, "Did you mean " + suggestions[0] + "?", curr_token->str_utf8)));
				}
			}

		}

		return results;
	}

	template<int FACTORS, int ORDER>
	string Spellchecker2<FACTORS, ORDER>::CheckText(const string &text)
	{
		u16string u_text = MyUtils::utf8_to_utf16(text);
		vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);

		u16stringReplacing usr = u16stringReplacing(u_text);

		int position_offset = 0;
		for (auto it = tokens.begin(); it != tokens.end(); it++)
		{
			vector<StagePosibility2<FACTORS>> stage_pos = decoder->DecodeTokenizedSentence(*it);

			for (uint i = 0; i < (*it).size(); i++)
			{
				if (stage_pos[i + ORDER - 1].is_original == false)
				{
						 
					TokenP token = (*it)[i];
					StagePosibility2<FACTORS> st_pos = stage_pos[i + ORDER - 1];
					usr.Replace(token->first, token->length, st_pos->word);
				}
			}
				 
		}

		return MyUtils::utf16_to_utf8(usr.GetResult());
	}

	template<int FACTORS, int ORDER>
	string Spellchecker2<FACTORS, ORDER>::DecodeEvaluation(const string &text, uint32_t num_sugg_to_output)
	{
			stringstream ret;
			u16string u_text = MyUtils::utf8_to_utf16(text);

			vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

			vector<StagePosibility2<FACTORS>> spv;
			vector<StagePosibility2<FACTORS>> stage_posibs;
			vector<uint> stage_posibs_first_ind;

			for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
			{
				vector<TokenP> &tokens = *it;
				decoder.DecodeTokenizedSentence_ReturnStagePosibilities(tokens, spv, stage_posibs, stage_posibs_first_ind);

				map<uint32_t, vector<StagePosibility2<FACTORS>>> sugg = MakeSuggestionList(spv, stage_posibs, stage_posibs_first_ind);

					
				for (uint32_t i = 0; i < tokens.size(); i++)
				{
					if (i > 0) ret << " ";

					auto fit = sugg.find(i + ORDER - 1); //!!!!!! i + decoder_order - 1

					if (fit == sugg.end())
					{
						if (spv[i + ORDER - 1].IsUnknown()) //!!!i + decoder_order - 1
						{
							//cerr << "spv->IsUnknown: " << tokens[i].GetWordStringISO() << endl;
							ret << tokens[i]->str_utf8 << "(spelling[])";
						}
						else
							ret << tokens[i]->str_utf8;
					}
					else
					{
						ret << tokens[i]->str_utf8;

						if (tokens[i]->isUnknown())
							ret << "(spelling[";
						else
							ret << "(grammar[";

						vector<StagePosibility2<FACTORS>> vec_sp = fit->second;

						//assert(vec_sp.size() > 0);

						for (uint32_t g = 0; g < min(vec_sp.size(), (size_t)num_sugg_to_output); g++)
						{
							if (g > 0) ret << "|";
							ret << MyUtils::utf16_to_utf8(vec_sp[g].word);
						}

						ret << "])";
					}

				}
			}


		return ret.str();
	}

	template<int FACTORS, int ORDER>
	string Spellchecker2<FACTORS, ORDER>::command_line_mode(const string &text, uint32_t num_sugg_to_output)
	{
			stringstream ret;

			u16string u_text = MyUtils::utf8_to_utf16(text);

			vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

			vector<StagePosibility2<FACTORS>> spv;
			vector<StagePosibility2<FACTORS>> stage_posibs;
			vector<uint> stage_posibs_first_ind;

			u16stringReplacing usr = u16stringReplacing(u_text);

			for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
			{
				vector<TokenP> &tokens = *it;
				decoder.DecodeTokenizedSentence_ReturnStagePosibilities(tokens, spv, stage_posibs, stage_posibs_first_ind);

				map<uint32_t, vector<StagePosibility2<FACTORS>>> sugg = MakeSuggestionList(spv, stage_posibs, stage_posibs_first_ind);

					
				for (uint32_t i = 0; i < tokens.size(); i++)
				{
					auto fit = sugg.find(i + ORDER - 1); //!!! i + decoder_order - 1

					if (fit == sugg.end())
					{
						if (spv[i + ORDER - 1].IsUnknown()) //!!! i + decoder_order - 1
						{
							u16string us = MyUtils::utf8_to_utf16("<spelling original=\"");
							us += tokens[i]->str_u16;
							us += MyUtils::utf8_to_utf16("\" suggestion=\"\"/>");
							//cerr << "spv->IsUnknown: " << tokens[i].GetWordStringISO() << endl;
							usr.Replace(tokens[i]->first, tokens[i]->length, us);
						}
							
					}
					else
					{
						u16string us;
						if (tokens[i]->isUnknown())
							us += MyUtils::utf8_to_utf16("<spelling original=\"");
						else
							us.append(MyUtils::utf8_to_utf16("<grammar original=\""));

						us.append(tokens[i]->str_u16);
						
						us.append(MyUtils::utf8_to_utf16("\" suggestions=\""));

						vector<StagePosibility2<FACTORS>> vec_sp = fit->second;

						//assert(vec_sp.size() > 0);

						for (uint32_t g = 0; g < min(vec_sp.size(), (size_t)num_sugg_to_output); g++)
						{
							if (g > 0) us.append(MyUtils::utf8_to_utf16("|"));
							us.append(vec_sp[g].word);
						}

						us.append(MyUtils::utf8_to_utf16("\"/>"));

						usr.Replace(tokens[i]->first, tokens[i]->length, us);
					}

				}

			}

			return MyUtils::utf16_to_utf8(usr.GetResult());
	}

	template<int FACTORS, int ORDER>
	Spellchecker2<FACTORS, ORDER>::Spellchecker2(Configuration* _configuration):
	configuration(_configuration), decoder(_configuration)
	{}

}

uint bleble(Configuration* _conf)
{
	Spellchecker2<3, 3> spellchecker(_conf);
	return 0;
}
