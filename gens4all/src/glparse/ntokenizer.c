#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "ntokenizer.h"

static size_t memspne(const char *searcharea, const char *end, const char *want) {
	size_t s = 0;
	while(searcharea < end) {
		if (strchr(want, *searcharea++) == NULL)
			return s;
		
		s++;
	}
	return s;
}
static size_t memcspne(const char *searcharea, const char *end, const char *want) {
	size_t s = 0;
	
	while(searcharea < end) {
		if (strchr(want, *searcharea++) != NULL)
			return s;
		s++;
		
	}
	return s;
}

void tokInit(tokState *t, const char * text, size_t len, const char *whitespace, const char *delim) {
	tokParseFrom(t, text, len);
	t->whitespace = whitespace;
	t->delim = delim;
}

void tokNext(tokState *t) {
	if (tokAtEnd(t)) {
		tokTokenLen(t) = 0;
		return;
	}
	
	//~ tokSavePosition(t, &t->last);
	
	const char *end_of_token = tokToken(t) + tokTokenLen(t);
	size_t start_ofs = memspne(end_of_token, t->pos.end, t->whitespace);
	size_t len = memcspne(end_of_token + start_ofs, t->pos.end, t->delim);
	
	//if we find a zero length token, then it's something not in WHITESPACE, but is in DELIM
	//so has to be it's something like a ( or a " or a null terminator
	//we want to pick these up, so set length to 1
	//this assumes sane WHITESPACE and DELIM
	if (len == 0)
		len = 1;
	
	//~ printf("SO: %i, l: %i\n", (int)start_ofs, (int)len);
	
	tokToken(t) += tokTokenLen(t) + start_ofs;
	tokTokenLen(t) = len;
}

void tokNextChar(tokState *t) {
	if (tokAtEnd(t)) {
		tokTokenLen(t) = 0;
		return;
	}
	
	//~ tokSavePosition(t, &t->last);
	
	const char *end_of_token = tokToken(t) + tokTokenLen(t);
	
	tokToken(t) = end_of_token;
	tokTokenLen(t) = 1;
}

void tokSkipTo(tokState *t, const char *skipchars) {
	size_t skipdistance = memcspne(tokToken(t), t->pos.end, skipchars);
	
	//~ printf("Skipped: %.*s\n", (int)(skipdistance , tokToken(t));
	
	tokToken(t) += skipdistance;
	tokTokenLen(t) = 0;
}

int tokEquals(tokState *t, const char *str) {
	if (tokTokenLen(t) == 0 || tokAtEnd(t))
		return 0;
	
	return strlen(str) == tokTokenLen(t) && memcmp(tokToken(t), str, tokTokenLen(t)) == 0;
}

int tokEqualsChar(tokState *t, char c) {
	return tokTokenLen(t) == 1 && tokToken(t)[0] == c;
}

int tokEqualsOneOfChar(tokState *t, const char *chars) {
	return tokTokenLen(t) == 1 && strchr(chars, tokToken(t)[0]) != NULL;
}

int tokNextCharEquals(tokState *t, char c) {
	const char *end_of_token = tokToken(t) + tokTokenLen(t);
	if (end_of_token >= t->pos.end)
		return 0;
	
	return tokToken(t)[tokTokenLen(t)] == c;
}

int tokNextCharIsOneOf(tokState *t, const char *chars) {
	const char *end_of_token = tokToken(t) + tokTokenLen(t);
	if (end_of_token >= t->pos.end)
		return 0;
	return strchr(chars, *end_of_token) != NULL;
}

int tokNextTokenEquals(tokState *t, const char *str) {
	tokPosition curpos;
	tokSavePosition(t, &curpos);
	int equals = tokEquals(t, str);
	tokRestorePosition(t, &curpos);
	return equals;
}

void tokParseFrom(tokState *t, const char *text, size_t len) {
	tokToken(t) = text;
	tokTokenLen(t) = 0;
	t->pos.end = text + len;
}

void tokSavePosition(tokState *t, tokPosition *dst) {
	memcpy(dst, &t->pos, sizeof(*dst));
}


void tokRestorePosition(tokState *t, tokPosition *src) {
	memcpy(&t->pos, src, sizeof(*src));
}

