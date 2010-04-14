/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#import "michalisekSpellChecker.h"

@implementation michalisekSpellChecker
-(NSPanel*)spellingPanel {
    NSPanel *p=[super spellingPanel];
    return p;
}

-(BOOL)checkWord:(NSString*)word {
	return true;
}

@end
