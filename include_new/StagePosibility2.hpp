#pragma once

#include "StdAfx.h"

template<int FACTORS>
struct StagePosibility2
{
	uint factors[FACTORS];
	u16string word;
	float emission_prob;
	bool is_original;

	bool IsUnknown()
	{
		return factors[0] == MyConstants::unknown_word_id || factors[0] == MyConstants::name_id;
	}
};