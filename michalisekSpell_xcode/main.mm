/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/


#import <Cocoa/Cocoa.h>
#import <iostream>
#import "delegate_Splitted.h"
#import "AddDiacriticsDelegate.h"
#import "delegate_joined.h"
#import "../include/Configuration.hpp"
#import "../include/Morphology.hpp"
#import "../include/Lexicon.hpp"
#import "../include/ErrorModelBasic.hpp"
#import "../include/StdAfx.h"
#import "../include/SimWordsFinder.hpp"
#import "../include/constants.hpp"
#import "../include/ZipLM.hpp"
#import "../include/LMWrapper.hpp"
#import "../include/Tokenizer.hpp"

void GetConfigurations(ConfigurationP &conf_spellchecking, ConfigurationP &conf_diacritics)
{    
	NSBundle *my_bundle = [NSBundle mainBundle];
	
	if (my_bundle == nil)
	{
		NSLog(@"FatalError: [NSBundle mainBundle] returns nil!");
	}
    
    MyConstants::isMacOS = true;

    conf_spellchecking = ConfigurationP(new Configuration());
    conf_spellchecking->viterbi_order = 3;
    
    conf_diacritics = ConfigurationP(new Configuration());
    conf_diacritics->viterbi_order = 3;
    
    string uc_to_lc_file = [[my_bundle pathForResource:@"uc_to_lc" ofType:@"dat"] UTF8String];
    string lc_to_uc_file = [[my_bundle pathForResource:@"lc_to_uc" ofType:@"dat"] UTF8String];
    string char_types_file = [[my_bundle pathForResource:@"char_types" ofType:@"dat"] UTF8String];
    
	MyUTF::init_mapping(uc_to_lc_file, lc_to_uc_file, char_types_file);

    string morpholex_file = [[my_bundle pathForResource:@"morphology" ofType:@"bin"] UTF8String];
    string error_model_file_spellchecking = [[my_bundle pathForResource:@"error_model_train0" ofType:@"bin"] UTF8String];
    string error_model_file_diacritics = [[my_bundle pathForResource:@"error_model_diacritics" ofType:@"bin"] UTF8String];

    string form_lm_file = [[my_bundle pathForResource:@"form_lm" ofType:@"bin"] UTF8String];
    string lemma_lm_file = [[my_bundle pathForResource:@"lemma_lm" ofType:@"bin"] UTF8String];
    string tag_lm_file = [[my_bundle pathForResource:@"tag_lm" ofType:@"bin"] UTF8String];
    string short_tag_lm_file = [[my_bundle pathForResource:@"short_tag_lm" ofType:@"bin"] UTF8String];
    
    ifstream ifs;
    ifs.open(morpholex_file.c_str(), ios::binary);
    if (ifs.is_open() == false)
    {
        cerr << "Can't open morpholex file (" << morpholex_file << ")" << endl;
        exit(-1234);
    }
    
    MorphologyP morphology = MorphologyP(new Morphology(ifs));
    LexiconP lexicon = LexiconP(new Lexicon(ifs));
    
    conf_spellchecking->LoadMorphologyAndLexicon(lexicon, morphology);
    conf_diacritics->LoadMorphologyAndLexicon(lexicon, morphology);
    ifs.close();
    
    ErrorModelBasicP emb_spellchecking = ErrorModelBasic::fromBinaryFile(error_model_file_spellchecking);
    conf_spellchecking->errorModel = emb_spellchecking;

    ErrorModelBasicP emb_diacritics = ErrorModelBasic::fromBinaryFile(error_model_file_diacritics);
    conf_diacritics->errorModel = emb_diacritics;

    
    
    ZipLMP form_lm = ZipLMP(new ZipLM(form_lm_file));
    LMWrapperP form_lm_wrapper = LMWrapperP(new LMWrapper(form_lm, 5000, 5000));
    conf_spellchecking->LoadLM(form_lm_wrapper);
    conf_diacritics->LoadLM(form_lm_wrapper);
    
    conf_spellchecking->EnableFactor(form_lm->GetFactorName(), 0.4f, 3);
    conf_diacritics->EnableFactor(form_lm->GetFactorName(), 0.4f, 3);
    
    ZipLMP lemma_lm = ZipLMP(new ZipLM(lemma_lm_file));
    LMWrapperP lemma_lm_wrapper = LMWrapperP(new LMWrapper(lemma_lm, 5000, 5000));
    conf_spellchecking->LoadLM(lemma_lm_wrapper);
    conf_diacritics->LoadLM(lemma_lm_wrapper);
    
    conf_spellchecking->EnableFactor(lemma_lm->GetFactorName(), 0.1f, 3);
    conf_diacritics->EnableFactor(lemma_lm->GetFactorName(), 0.1f, 3);

    ZipLMP tag_lm = ZipLMP(new ZipLM(tag_lm_file));
    LMWrapperP tag_lm_wrapper = LMWrapperP(new LMWrapper(tag_lm, 5000, 5000));
    conf_spellchecking->LoadLM(tag_lm_wrapper);
    conf_diacritics->LoadLM(tag_lm_wrapper);
    
    conf_spellchecking->EnableFactor(tag_lm->GetFactorName(), 0.5f, 3);
    conf_diacritics->EnableFactor(tag_lm->GetFactorName(), 0.5f, 3);

	vector<SimWordsFinder::SearchConfig> search_configs;

    search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::case_sensitive, 1, 6));
    search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 1, 6));
    search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 2, 9));

	vector<SimWordsFinder::SearchConfig> search_configs_diacritics;
    
    search_configs_diacritics.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::case_sensitive, 8, 8));
    search_configs_diacritics.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 8, 8));

	conf_spellchecking->st_pos_multifactor_cash = MyCash_StagePosibilityP(new MyCash_StagePosibility(5000, 20)); 
	conf_diacritics->st_pos_multifactor_cash = MyCash_StagePosibilityP(new MyCash_StagePosibility(5000, 20));
    
	TokenizerP tokenizer = TokenizerP(new Tokenizer() );
	tokenizer->initLexicon(lexicon);
	conf_spellchecking->tokenizer = tokenizer;
    
	SimWordsFinderP swf_spellchecking = SimWordsFinderP(new SimWordsFinder(conf_spellchecking.get(), search_configs));
	SimWordsFinderP swf_diacritics = SimWordsFinderP(new SimWordsFinder(conf_diacritics.get(), search_configs_diacritics));
    
	conf_spellchecking->simWordsFinder = swf_spellchecking;
    conf_diacritics->simWordsFinder = swf_diacritics;
}


int main(int argc, char *argv[])
{
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	ConfigurationP conf_spellchecking;
    ConfigurationP conf_diacritics;
    
    GetConfigurations(conf_spellchecking, conf_diacritics);
	
	AddDiacriticsDelegate *add_del = [[AddDiacriticsDelegate alloc] init];
    [add_del setConfiguration:conf_diacritics];
    
	[add_del autorelease]; 
	
    //NSString* ble = [NSString stringWithUTF8String:"olala"];
    
	NSRegisterServicesProvider(add_del, @"addDiacritics");
	NSRegisterServicesProvider(add_del, @"removeDiacritics");
	
	//[[NSRunLoop currentRunLoop] acceptInputForMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:30.0]];
	
    delegate_Splitted *spellchecking_delegate = [[delegate_Splitted alloc] init];
    [spellchecking_delegate setConfiguration:conf_spellchecking];
    
    [spellchecking_delegate autorelease];
    
	NSSpellServer *aServer = [[[NSSpellServer alloc] init] autorelease];
	if ([aServer registerLanguage:@"cs_CZ" byVendor:@"michalisekSpell"]) {
		[aServer setDelegate:spellchecking_delegate ];
		[aServer run];
		NSLog(@"Unexpected death of NgramSpellChecker!\n");
	}
	else {
		NSLog(@"Unable to check in GramSpellChecker.\n");
	}
	
	[pool release];
	return 0;
	
	
}
