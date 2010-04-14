/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#import <Cocoa/Cocoa.h>


@interface AddDiacriticsDelegate : NSObject {
	ConfigurationP configuration;
}

- (void)addDiacritics:(NSPasteboard *)pboard userData:(NSString *)userData error:(NSString **)error;
- (void)removeDiacritics:(NSPasteboard *)pboard userData:(NSString *)userData error:(NSString **)error;
- (void)setConfiguration:(ConfigurationP)_configuration;

@end

@implementation AddDiacriticsDelegate

- (void)setConfiguration:(ConfigurationP)_configuration
{
    configuration = _configuration;
}

- (void)addDiacritics:(NSPasteboard *)pboard userData:(NSString *)userData error:(NSString **)error
{
	NSString *pboardString;
	NSArray *types;
	
	//logString("catUpXML executed!");
	
	types = [pboard types];
	if (![types containsObject:NSStringPboardType]) {
		*error = NSLocalizedString(@"Error: couldn't encrypt text.",
								   @"pboard couldn't give string.");
		return;
	}
	pboardString = [pboard stringForType:NSStringPboardType];
	if (!pboardString) {
		*error = NSLocalizedString(@"Error: couldn't encrypt text.",
								   @"pboard couldn't give string.");
		return;
	}
	
	//NSError *convertError = nil;
	
	//string pboard_cpp_string = [pboardString UTF8String];
	string pboard_cpp_string = [pboardString UTF8String];
		
    Spellchecker spellchecker(configuration.get());
    string new_cpp_string = spellchecker.CheckText(pboard_cpp_string);
	NSString* newString = [NSString stringWithUTF8String: new_cpp_string.c_str()];
	if (! newString) {
		*error = NSLocalizedString(@"Error: conversion to UTF error.",
								   @"pboard couldn't give string.");
		NSLog(@"AddDiacritics: conversion to UTF error!");
		return;
	}
	
	types = [NSArray arrayWithObject:NSStringPboardType];
	[pboard declareTypes:types owner:nil];
	[pboard setString:newString forType:NSStringPboardType];
	//NSLog(@"AddDiacritics finished succesfully!");
	return;
}

- (void)removeDiacritics:(NSPasteboard *)pboard userData:(NSString *)userData error:(NSString **)error
{
	NSString *pboardString;
	NSString *newString;
	NSArray *types;
	
	//NSLog(@"Remove diacritics started!");
	
	types = [pboard types];
	if (![types containsObject:NSStringPboardType]) {
		*error = NSLocalizedString(@"Error: couldn't encrypt text.",
								   @"pboard couldn't give string.");
		return;
	}
	pboardString = [pboard stringForType:NSStringPboardType];
	if (!pboardString) {
		*error = NSLocalizedString(@"Error: couldn't encrypt text.",
								   @"pboard couldn't give string.");
		return;
	}
	
	
	string pboard_cpp_string = [pboardString UTF8String];
	
    //TODO: reimplement functionality
	string new_cpp_string = pboard_cpp_string;/* = GlobalVars::textCheckingServer->RemoveDiacritics(pboard_cpp_string_iso);*/
	
	newString = [NSString stringWithUTF8String: new_cpp_string.c_str()];
	if (! newString) {
		*error = NSLocalizedString(@"Error: Conversion to UTF8 failed.",
								   @"pboard couldn't give string.");
		NSLog(@"Remove Diacritics Conversion Error!");
		return;
	}
	
	types = [NSArray arrayWithObject:NSStringPboardType];
	[pboard declareTypes:types owner:nil];
	[pboard setString:newString forType:NSStringPboardType];
	
	//NSLog(@"Remove Diacritics Finished!");
	return;
}


@end
