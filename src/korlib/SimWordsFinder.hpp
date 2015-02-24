/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef SIM_WORDS_FINDER_HPP_
#define SIM_WORDS_FINDER_HPP_

#include "StdAfx.h"

namespace ngramchecker {
	
	struct Token;
	SP_DEF(Token);

	class Configuration;

	class SimWordsFinder {
	public:

		enum casing_treatment { case_sensitive, ignore_case_keep_orig, ignore_case };

		struct SearchConfig
		{
			casing_treatment casing;
			uint max_ed_dist;
			float max_cost;

			SearchConfig(casing_treatment _casing, uint _max_ed_dist, float _max_cost):
				casing(_casing), max_ed_dist(_max_ed_dist), max_cost(_max_cost) {}
		};

	private:
		Configuration* configuration;
		vector<SearchConfig> search_configs;

		void Find_basic(const TokenP &token, uint32_t lookup_max_ed_dist, double lookup_max_cost, Similar_Words_Map &ret);

		void Find_basic_ignore_case(const TokenP &token, bool keep_orig_casing, uint32_t lookup_max_ed_dist, double lookup_max_cost, Similar_Words_Map &ret);

	public:
			SimWordsFinder(Configuration* _configuration, const vector<SearchConfig> &_search_configs):
				configuration(_configuration), search_configs(_search_configs)
			{}

			Similar_Words_Map Find(const TokenP &token);

			vector<SearchConfig> GetSearchConfigs() { return search_configs; }
	};

	SP_DEF(SimWordsFinder);
}

#endif /* SIM_WORDS_FINDER_HPP_ */
