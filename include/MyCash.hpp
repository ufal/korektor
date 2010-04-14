/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef STAGEPOSIBILITY_CASH_HPP_
#define STAGEPOSIBILITY_CASH_HPP_

//#include "StdAfx.h"

#include <unordered_map>
#include <list>
#include <vector>
#include <stdint.h>

using namespace std;


namespace ngramchecker {

	template <class key_type, class value_type, typename Hash = std::hash<key_type>, typename Pred = std::equal_to<key_type> >
	class MyCash
	{
	private:

		//typedef pair<string, bool> key_type;
		//typedef vector<StagePosibilityP> value_type;


		typedef list<key_type > list_type;
		typedef typename list_type::iterator list_iterator_type;
		typedef  unordered_map<key_type, pair<value_type, list_iterator_type>, Hash, Pred > map_type;
		typedef typename map_type::iterator map_iterator_type;

		map_type sp_map;
		list<key_type> stack;

		uint32_t num_stored_values;

		uint32_t max_values;
		uint32_t push_front_period;
		uint32_t counter;
	public:

		MyCash(uint32_t _max_values, uint32_t _push_front_period = 5):num_stored_values(0), max_values(_max_values), push_front_period(_push_front_period), counter(0)
		{
		}
		MyCash() {}
		void StoreValueForKey(key_type key, value_type value)
		{
			//assert(sp_map.find(key) == sp_map.end());

			stack.push_front(key);

			sp_map[key] = pair<value_type, list_iterator_type>(value, stack.begin());

			if (num_stored_values < max_values)
			{
				num_stored_values++;
			}
			else
			{
				list_iterator_type eit = stack.end();
				eit--;
				sp_map.erase( (*eit) );
				stack.erase(eit);
			}
		}


		bool IsCashed(key_type key)
		{
			return sp_map.find(key) != sp_map.end();
		}

		value_type GetCashedValue(key_type key)
		{

			map_iterator_type fit = sp_map.find(key);

			//assert(fit != sp_map.end());

			//counter++;
			//if (counter % push_front_period == 0)
			//{
			//	counter = 0;
				list_iterator_type lit = fit->second.second;
				stack.erase(lit);
				stack.push_front(key);
				sp_map[key].second = stack.begin();
			//}

			return fit->second.first;
		}

	};

}

#endif /* STAGEPOSIBILITY_CASH_HPP_ */
