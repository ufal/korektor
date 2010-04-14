/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "../include/StdAfx.h"
#import "../include/Configuration.hpp"
#import "../include/Spellchecker.hpp"
#import "../include/SimWordsFinder.hpp"
#import "../include/Lexicon.hpp"
#import "../include/TextCheckingResult.hpp"

@interface delegate_Splitted : NSObject <NSSpellServerDelegate> {
	
    ConfigurationP configuration;
    
}

- (NSRange)spellServer:(NSSpellServer *)sender findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly;

- (NSRange)spellServer:(NSSpellServer *)sender checkGrammarInString:(NSString *)stringToCheck language:(NSString *)language details:(NSArray **)details;

- (NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language;

- (void)setConfiguration:(ConfigurationP)_configuration;

@end


@implementation delegate_Splitted

- (void)setConfiguration:(ConfigurationP)_configuration
{
    configuration = _configuration;
}

- (NSRange)spellServer:(NSSpellServer *)sender findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly {
	
	
	//NSLog(@"%@", [NSString stringWithFormat:@"Find misspelledWord: %@", stringToCheck]);
	
	*wordCount = -1;
		
	uint32_t range_from;
	uint32_t range_length;
	
    string cpp_string = [stringToCheck UTF8String];
    
    Spellchecker spellchecker(configuration.get());
    spellchecker.FindMisspelledWord(cpp_string, range_from, range_length);
    
    //TODO: reimplement FindMisspelledWord
	//GlobalVars::textCheckingServer->FindMisspelledWord(cpp_stringISO, range_from, range_length);
	
	if (range_length == 0) {
		return NSMakeRange(NSNotFound, 0);
	}
	else {
		return NSMakeRange(range_from, range_length);
	}
	
	
}

- (NSRange)spellServer:(NSSpellServer *)sender checkGrammarInString:(NSString *)stringToCheck language:(NSString *)language details:(NSArray **)details {
	
	//NSLog(@"Check grammar in string: %@", stringToCheck);
	
	NSMutableArray* ret_details = [NSMutableArray array];
	
	string cpp_string = [stringToCheck UTF8String];
	
	uint32_t range_from, range_length;
    Spellchecker spellchecker(configuration.get());
    //TODO: change method so that it returns ranges as well!
    vector<TextCheckingResultP> tchr = spellchecker.GetCheckingResultsFirstSentence(cpp_string, range_from, range_length);
	
	for (vector<TextCheckingResultP>::iterator it = tchr.begin(); it != tchr.end(); it++)
	{
		//logString("tchr->begin");
		NSString* err_desc;
		
		TextCheckingResultP one_res = *it;
		
		if (one_res->type == grammar)
		{
			//stringstream errorInfo;
			//errorInfo << "Grammar error: (";
			//errorInfo << one_res->range_from << ", " << one_res->range_length << ")";
			//logString(one_res->ToString().c_str());
			
			
			GrammarCheckingResultP gr = std::static_pointer_cast<GrammarCheckingResult>(one_res);
			
			//logString("Conversion to GrammarCheckingRes OK!");
			
			string utf_err_desc = gr->error_description;
			err_desc = [NSString stringWithUTF8String:utf_err_desc.c_str()];
			//suggestion = [NSString stringWithUTF8String:gr->suggestion.c_str()];
			
			NSMutableArray* sugg_array = [NSMutableArray array];
			
			for (size_t i = 0; i < gr->suggestions.size(); i++)
			{
				string utf_sugg = gr->suggestions[i];
				[sugg_array addObject:[NSString stringWithUTF8String:utf_sugg.c_str()]]; 
			}
			
			//logString("sugg_array OK!");
			
			NSArray* keys = [NSArray arrayWithObjects:NSGrammarRange, NSGrammarUserDescription, NSGrammarCorrections, nil];
			NSArray* values = [NSArray arrayWithObjects:[NSValue valueWithRange:NSMakeRange(one_res->range_from, one_res->range_length)], err_desc, sugg_array, nil];
			
			NSDictionary* dict = [NSDictionary dictionaryWithObjects:values forKeys:keys];
			[ret_details addObject:dict];
			
			//logString("error info added to dictionary!");
			
		}
		
	}
	
	*details = ret_details; 
	
	//NSLog(@"Grammar check finished!");
	
	return NSMakeRange(range_from, range_length);	
}

- (NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language {
	
	//NSLog(@"%@ : %@", @"Suggest gueses for word:", word);
	
	NSMutableArray* sugg = [NSMutableArray array];
	
	string word_cpp = [word UTF8String];
	
    Spellchecker spellchecker(configuration.get());
    vector<string> sugg_cpp = spellchecker.GetContextFreeSuggestions(word_cpp);
    //TODO: reimplement this functionality!
	//vector<string> sugg_cpp = GlobalVars::textCheckingServer->GetSuggestions(word_cpp_iso);
	
	
	for (size_t i = 0; i < sugg_cpp.size(); i++)
	{
		NSString *single_guess = [NSString stringWithUTF8String:sugg_cpp[i].c_str()];
		//NSLog(@"%@ : %@", @"guess:", single_guess);
		[sugg addObject:single_guess];
	}
	
	return sugg;
}

@end
