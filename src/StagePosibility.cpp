/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "StagePosibility.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

namespace ngramchecker {


	string StagePosibilityNew::ToString()
	{
		return MyUtils::utf16_to_utf8(word);
	}

	bool StagePosibilityNew::IsUnknown()
	{
		return form_id == MyConstants::unknown_word_id || form_id == MyConstants::name_id;
	}

	StagePosibilityNew::StagePosibilityNew(const FactorList &_factorList, bool _original, const u16string &_word, Configuration* _conf, float error_model_cost):
		StagePosibility(MultiFactor), word(_word), original(_original), emission_prob(error_model_cost), factorList(_factorList) 
	{

		uniq_id = 0;

		uint num_factors = _conf->NumFactors();

		for (uint j = 0; j < num_factors; j++)
		{
			if (_conf->IsFactorEnabled(j))
			{
				if (j > 0)
					emission_prob += _conf->GetFactorWeight(j) * factorList.emission_costs[j];
				MyUtils::HashCombine(uniq_id, factorList.factors[j]);
			}
		}

		form_id = factorList.factors[0];
	}

	/*	StagePosibility_Letter::StagePosibility_Letter(UChar _letter):
			StagePosibility(Letter), letter(_letter) {}

		double StagePosibility_Letter::EmmisionProbability()
		{
			return 0.0;
		}

		string StagePosibility_Letter::ToString()
		{
			assert(1 == 0);
			return "";
		}

		bool StagePosibility_Letter::IsOriginal()
		{
			return false;
		}

		uint32_t StagePosibility_Letter::UniqueIdentifier()
		{
			return letter;
		}

		uint32_t StagePosibility_Letter::FormIdentifier()
		{
			return letter;
		}

		bool StagePosibility_Letter::IsUnknown()
		{
			return false;
		}

		uint32_t StagePosibility_Letter::Letter()
		{
			return letter;
		}

		StagePosibilityP StagePosibility_Letter::SentenceStartSP()
		{
			return StagePosibilityP(new StagePosibility_Letter(GlobalVars::stringMapper->MapChar('.'), 0));
		}

		StagePosibilityP StagePosibility_Letter::SentenceEndSP()
		{
			return StagePosibilityP(new StagePosibility_Letter(GlobalVars::stringMapper->MapChar(','), 0));
		}*/
}
