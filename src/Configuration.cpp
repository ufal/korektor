#include "StdAfx.h"
#include "Configuration.hpp"

#include "Morphology.hpp"
#include "Lexicon.hpp"
#include "Tokenizer.hpp"
#include "LMWrapper.hpp"
#include "SimWordsFinder.hpp"
#include "ErrorModelBasic.hpp"
#include "ZipLM.hpp"
#include "LMWrapper.hpp"

namespace ngramchecker {

	void Configuration::LoadMorphologyAndLexicon(LexiconP _lexicon, MorphologyP _morphology)
	{
		lexicon = _lexicon;
		morphology = _morphology;
		
		factor_map = morphology->GetFactorMap();
		
		for (auto it = factor_map.begin(); it != factor_map.end(); it++)
			factor_names.push_back("");

		for (auto it = factor_map.begin(); it != factor_map.end(); it++)
			factor_names[it->second] = it->first;

		for (auto it = factor_names.begin(); it != factor_names.end(); it++)
		{
				enabled_factors.push_back(false);
				factor_weights.push_back(0.0f);
				factor_LMS.push_back(LMWrapperP());
				factor_orders.push_back(0);
		}

	}

	void Configuration::LoadLM(LMWrapperP lm)
	{
		uint index = factor_map[lm->FactorName()];
		factor_LMS[index] = lm;
	}

	void Configuration::EnableFactor(const string &fac_name, float weight, uint order)
	{
		uint index = factor_map[fac_name];
		enabled_factors[index] = true;
		factor_weights[index] = weight;
		factor_orders[index] = order;

		if (index > last_enabled_factor_index)
			last_enabled_factor_index = index;
	}


};

Configuration::Configuration(const string &exec_name, const string &conf_file)
{
	last_enabled_factor_index = 0;
	diagnostics = false;

	/*size_t slash_pos = exec_name.rfind('\\');
	size_t slash2_pos = exec_name.rfind('/');*/

	size_t slash_pos = conf_file.rfind('\\');
	size_t slash2_pos = conf_file.rfind('/');

	if (slash_pos == string::npos && slash2_pos == string::npos)
	{
		//slash_pos = 0;
		//exit(1);
		//throw std::bad_exception("Can't determine the application directory!");
	}

	string application_directory;

	/*if (slash_pos == string::npos)
		application_directory = exec_name.substr(0, slash2_pos + 1);
	else if (slash2_pos == string::npos)
		application_directory = exec_name.substr(0, slash_pos + 1);
	else
		application_directory = exec_name.substr(0, max(slash_pos, slash2_pos + 1));

	cerr << "Application directory: " << application_directory << endl;*/

	if (slash_pos == string::npos && slash2_pos == string::npos)
		application_directory = "";
	else if (slash_pos == string::npos)
		application_directory = conf_file.substr(0, slash2_pos + 1);
	else if (slash2_pos == string::npos)
		application_directory = conf_file.substr(0, slash_pos + 1);
	else
		application_directory = conf_file.substr(0, max(slash_pos, slash2_pos + 1));

	cerr << "Application directory: " << application_directory << endl;


	ifstream ifs;
	//string conf_file_abs = application_directory + conf_file;
	ifs.open((conf_file).c_str());

	if (ifs.is_open() == false)
	{
		cerr << "Opening configuration file " << conf_file << "failed!" << endl;
		exit(1);
		//throw std::bad_exception("Can't open configuration file!");
	}
	
	viterbi_order = 1;

#ifdef WIN32
	string data_directory = application_directory + "data\\";
#else
	string data_directory = application_directory + "data/";
#endif

	string error_model_file;// = data_directory + "error_model.bin";

	vector<SimWordsFinder::SearchConfig> search_configs;

	mode_string = "tag_errors";
	string s;

	while (MyUtils::SafeReadline(ifs, s))
	{
		if (s == "" || s[0] == '#')  //comment
			continue;

		if (s == "diagnostics")
		{
			diagnostics = true;
		}
		else if (s.substr(0, 9) == "morpholex")
		{
			string morpholex_file = data_directory + s.substr(10);
			ifstream ifs;
			ifs.open(morpholex_file.c_str(), ios::binary);
			if (ifs.is_open() == false)
			{
				cerr << "Opening morpholex file " << morpholex_file << " failed!" << endl;
				exit(1);
				//throw std::bad_exception("Can't open morpholex file!");
			}

			MorphologyP morphology = MorphologyP(new Morphology(ifs));
			LexiconP lexicon = LexiconP(new Lexicon(ifs));

			LoadMorphologyAndLexicon(lexicon, morphology);

			//cerr << "morpholex loaded!" << endl;
			
			ifs.close();
		}
		else if (s.substr(0, 10) == "errormodel")
		{
			string error_model_file = data_directory + s.substr(11);

			ErrorModelBasicP emb = ErrorModelBasic::fromBinaryFile(error_model_file);
			errorModel = emb;
			//cerr << "error model loaded" << endl;
		}
		else if (s.substr(0, 2) == "lm")
		{
			vector<string> toks;

			MyUtils::Split(toks, s, "-");

			FATAL_CONDITION(toks.size() == 4, s);

			ZipLMP lm = ZipLMP(new ZipLM(data_directory + toks[1]));
			LMWrapperP lm_wrapper = LMWrapperP(new LMWrapper(lm, 5000, 5000));
			LoadLM(lm_wrapper);

			string order_str = toks[2];
			uint order = MyUtils::my_atoi(order_str);
			
			string weight_str = toks[3];
			float weight = MyUtils::my_atof(weight_str);

			EnableFactor(lm->GetFactorName(), weight, order);

			if (viterbi_order < order)
				viterbi_order = order;

			//cerr << "language model " << toks[1] << " loaded!" << endl;
		}
		else if (s.substr(0, 6) == "search")
		{
			vector<string> toks;
			MyUtils::Split(toks, s, "-");

			FATAL_CONDITION(toks.size() == 4, s);

			SimWordsFinder::casing_treatment ct;

			if (toks[1] == "case_sensitive")
				ct = SimWordsFinder::case_sensitive;
			else if (toks[1] == "ignore_case")
				ct = SimWordsFinder::ignore_case;
			else if (toks[1] == "ignore_case_keep_orig")
				ct = SimWordsFinder::ignore_case_keep_orig;
			else
			{
				cerr << s << endl << toks[1] << " - invalid value of casing treatment" << endl;
				exit(1); 
			}

			uint max_edit_distance = MyUtils::my_atoi(toks[2]);
			float max_cost = MyUtils::my_atof(toks[3]);

			search_configs.push_back(SimWordsFinder::SearchConfig(ct, max_edit_distance, max_cost));
		}
		else if (s.substr(0, 4) == "mode")
		{
			mode_string = s.substr(5);

			if (mode_string != "autocorrect" && mode_string != "tag_errors")
			{
				cerr << s << endl << " - invalid value of output mode" << endl;
				exit(1);
			}
		}
	}

	st_pos_multifactor_cash = MyCash_StagePosibilityP(new MyCash_StagePosibility(5000, 20));

	tokenizer = TokenizerP(new Tokenizer() );
	tokenizer->initLexicon(lexicon);
	
	//search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::case_sensitive, 1, 6));
	//search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 1, 6));
	//search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 2, 9));

	simWordsFinder = SimWordsFinderP(new SimWordsFinder(this, search_configs));

	//TODO: morphology vocab file should be set in configuration file as well!
	if (diagnostics)
	{
		morphology->initMorphoWordLists(data_directory + "morphology_h2mor_freq2_vocab.bin");
		morphology->initMorphoWordMaps();
	}

	if (is_initialized() == false)
	{
		//throw std::bad_exception("configuration is not initialized!");
		cerr << "Configuration was not initialized proparly!" << endl;
		exit(1);
	}

}
