/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#import <Cocoa/Cocoa.h>


@interface delegate_joined : NSObject {

}

- (NSArray *)spellServer:(NSSpellServer *)sender checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount;

- (NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language;


@end

@implementation delegate_joined

- (NSArray *)spellServer:(NSSpellServer *)sender checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount {
		
	int next_word_start_index = 0;
	
	NSMutableArray* ma = [NSMutableArray array];
	
	size_t resultCount = 0;
	
	for (size_t i = 0; i < [stringToCheck length]; i++)
	{
		if ([stringToCheck characterAtIndex:i] == ' ' || [stringToCheck characterAtIndex:i] == '\n')
		{
			if (next_word_start_index < i)
			{
				if (resultCount % 2 == 0)
				{
					NSTextCheckingResult *res = [NSTextCheckingResult spellCheckingResultWithRange:NSMakeRange(next_word_start_index + offset, i - next_word_start_index)];
					[ma addObject:res];
				}
				else if (resultCount % 2 == 1)
				{
					NSArray* keys = [NSArray arrayWithObjects:NSGrammarRange, NSGrammarUserDescription, NSGrammarCorrections, nil];
					NSArray* values = [NSArray arrayWithObjects:[NSValue valueWithRange:NSMakeRange(/*next_word_start_index + offset*/0, i - next_word_start_index)], @"Je tam chyba, má tam být byvše!", [NSArray arrayWithObject:@"byvše"], nil];
					NSDictionary* dict = [NSDictionary dictionaryWithObjects:values forKeys:keys];
					
					NSTextCheckingResult *res = [NSTextCheckingResult grammarCheckingResultWithRange:NSMakeRange(next_word_start_index + offset, i - next_word_start_index) details:[NSArray arrayWithObject:dict]];
					
					NSString* replacement_str;
					if (rand() % 2 == 1)
					{
						replacement_str = @"[rep_1111]";
					}
					else {
						replacement_str = @"[rep_2222]";
					}
										
					[ma addObject:res];
					
				}
				
				resultCount++;
				
			}
			
			next_word_start_index = i + 1;
		}
	}
	
	*wordCount = resultCount;
	
	return ma;
	
}

- (NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language
{
	NSMutableArray* ret = [NSMutableArray array];
	[ret addObject:@"correction1"];
	[ret addObject:@"correction2"];
	return ret;
	
}

@end
