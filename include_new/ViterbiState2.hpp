#pragma once

#include "StdAfx.h"
#include "utils.hpp"
#include "StagePosibility2.hpp"



template<int FACTORS, int ORDER>
struct ViterbiState2
{

	uint offset[FACTORS][ORDER];
	uint num_entries[FACTORS][ORDER];
	float bow[FACTORS][ORDER];

	float distance;
	StagePosibility2<FACTORS> *stage_posibility;

	size_t hash_code;

	ViterbiState2<FACTORS, ORDER>* ancestor;

	

	bool Equals(const ViterbiState2<FACTORS, ORDER> &state)
	{
		for (uint i = 0; i < FACTORS; i++)
			for (uint j = 0; j < ORDER; j++)
				if (offset[i][j] != state.offset[i][j])
					return false;

		return true;
	}

	void ComputeHash()
	{
		hash_code = 0;
		for (uint i = 0; i < FACTORS; i++)
			for (uint j = 0; j < ORDER; j++)
				MyUtils::HashCombine(hash_code, offset[i][j]);
	}
};