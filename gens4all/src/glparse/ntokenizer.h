#ifndef NTOKENIZER_H
#define NTOKENIZER_H

#include <string.h>

#define tokToken(t) ((t)->pos.token)
#define tokTokenLen(t) ((t)->pos.tokenlen)
#define tokAtEnd(t) (tokToken(t) >= (t)->pos.end)
#define tokPass(t) tokToken(t), tokTokenLen(t)

//Includes current token as remaining
#define tokRemaining(t) ((t)->pos.end - tokToken(t))

//Usage: printf("%.*s", tokPrint(t));
#define tokPrint(t) ((int)tokTokenLen(t)), tokToken(t)

typedef struct {
	//Start of current token
	const char *token;
	
	//Length of current token
	size_t tokenlen;
	
	//Remaining bytes of string after current token
	//~ size_t remaining;
	const char *end;
	
	//~ int line,col;
} tokPosition;

typedef struct {
	tokPosition pos;
	const char * whitespace;
	const char * delim;
} tokState;

//Sets up a tokenizer to a specific string with whitespace and delimiter
//Need to call tokNext or tokNextChar once before reading it
void tokInit(tokState *t, const char * text, size_t len, const char *whitespace, const char *delim);

//Advances to next token after current one
//Skips whitespace and finds end of token based on delim
void tokNext(tokState *t);

//Gets next character after current token
//Does not skip whitespace
void tokNextChar(tokState *t);

//Finds next occurance of skipchars, starting with beginning of token (not end of token)
//If current token starts with skipchar, does not advance
//If current token contains skipchar, goes to first occurence
//Need to call tokNext or tokNextChar afterwards to get length of token
void tokSkipTo(tokState *t, const char *skipchars);

//Returns true if current token equals str, false otherwise
//If at end of string, returns false
int tokEquals(tokState *t, const char *str);

//Returns true if current token equals the character c, false otherwise
//If at end of string, returns false
int tokEqualsChar(tokState *t, char c);

int tokEqualsOneOfChar(tokState *t, const char *chars);

//Returns true if next character after current token equals character c, false otherwise
//If no next character, returns false
int tokNextCharEquals(tokState *t, char c);

int tokNextCharIsOneOf(tokState *t, const char *chars);

void tokParseFrom(tokState *t, const char *text, size_t len);
void tokSavePosition(tokState *t, tokPosition *dst);
void tokRestorePosition(tokState *t, tokPosition *src);

#endif
