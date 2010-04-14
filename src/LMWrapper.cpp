/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "LMWrapper.hpp"
#include "ZipLM.hpp"
#include "NGram.hpp"
#include "utils.hpp"

namespace ngramchecker {

		//!!! The ordering of IDs is not reversed anymore !!!
		void LMWrapper::GetNGram(NGram& ngram_key, NGram& ngram_ret)
		{
			size_t hash_key = ngram_hash_function(ngram_key);

			if (ngram_cash.IsCashed(hash_key))
			{
				triple(uint, double, double) trp = ngram_cash.GetCashedValue(hash_key);
				ngram_ret.order = FIRST(trp);
				ngram_ret.prob = SECOND(trp);
				ngram_ret.backoff = THIRD(trp);
				memcpy(ngram_ret.word_ids, ngram_key.word_ids, sizeof(uint32_t) * ngram_ret.order);
				return;
			}

			LM->GetNGramForNGramKey(ngram_key, ngram_ret);

			uint32_t start_undef = ngram_ret.order + 1;
			uint32_t end_undef = ngram_key.order;

			for (uint32_t undef_order = start_undef; undef_order <= end_undef; undef_order++)
			{
				ngram_key.order = undef_order;
				size_t undef_hash = ngram_hash_function(ngram_key);

				if (! ngram_cash.IsCashed(undef_hash))
					ngram_cash.StoreValueForKey(undef_hash, make_triple(0, 0.0, 0.0));
			}

			ngram_key.order = end_undef;

			if (ngram_ret.order > 0)
			{
				if (ngram_ret.order == ngram_key.order)
				{
					ngram_cash.StoreValueForKey(hash_key, make_triple(ngram_ret.order, ngram_ret.prob, ngram_ret.backoff));
				}
				else
				{
					WARNING(true, "This code shouldn't have been called in the current verion!");
					size_t ret_hash = ngram_hash_function(ngram_ret);
					if (! ngram_cash.IsCashed(ret_hash) )
						ngram_cash.StoreValueForKey(ret_hash, make_triple(ngram_ret.order, ngram_ret.prob, ngram_ret.backoff));
				}


			}

		}

		LMWrapper::LMWrapper(ZipLMP _LM, uint32_t ngram_cash_capacity, uint32_t ngram_probs_cash_capacity):
			LM(_LM),
			_lookups(0),
			_hits(0),
			ngram_cash(ngram_cash_capacity),
			ngram_probs_cash(ngram_probs_cash_capacity)
		{}

		double LMWrapper::GetProb(NGram& ngram_key, NGram& ngram_pom)
		{
			size_t key_hash = ngram_hash_function(ngram_key);

			if (ngram_probs_cash.IsCashed(key_hash))
			{
				return ngram_probs_cash.GetCashedValue(key_hash);
			}


			/*if (ngram_cash.IsCashed(key_hash))
			{
				triple(uint, double, double) trp = ngram_cash.GetCashedValue(key_hash);

				if (FIRST(trp) != 0)
				{
					return  SECOND(trp);
				}
			}*/

			GetNGram(ngram_key, ngram_pom);

			double ret_prob = ngram_pom.prob;

			if (ngram_key.order == ngram_pom.order)
				return ret_prob;

			int highest_bow_order = ngram_key.order - 1;
			int lowest_bow_order = std::max(1, (int)ngram_pom.order);

			uint ngram_orig_order = ngram_key.order;

			ngram_key.word_ids++;
			ngram_key.order = (uint)highest_bow_order;

			while ((int)ngram_key.order >= lowest_bow_order)
			{
				GetNGram(ngram_key, ngram_pom);

				if ((int)ngram_pom.order >= lowest_bow_order)
					ret_prob += ngram_pom.backoff;
					
				if (ngram_pom.order == 0)
					break;



				ngram_key.order = ngram_pom.order - 1;
			}

			ngram_probs_cash.StoreValueForKey(key_hash, ret_prob);

			ngram_key.word_ids--;
			ngram_key.order = ngram_orig_order;

			return ret_prob;

		}


		bool LMWrapper::ContainsNGram(NGram& ngram_key, NGram& ngram_pom)
		{
			GetNGram(ngram_key, ngram_pom);

			return (ngram_pom.order > 0);
		}

		void LMWrapper::ClearCash()
		{
			/*ngram_cash.;
			cashed_ids.clear();*/
		}

		void LMWrapper::PrintStats()
		{
			cerr << "LMWrapper_ZipLM \"" << LM->GetFilename() << "\" Stats:" << endl << "lookups = " << _lookups << endl << "hits = " << _hits << endl;
		}

		uint32_t LMWrapper::MaxUnigramID()
		{
			return LM->MaxUnigramID();
		}

		string LMWrapper::FactorName()
		{
			return LM->GetFactorName();
		}

		NGram_ihash LMWrapper::ngram_hash_function;
}
