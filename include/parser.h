#ifndef CTOY_PARSER_H
#define CTOY_PARSER_H

#include "scanner.h"
#include "chunk.h"
#include "object.h"
#include "common.h"

//local variables
typedef struct {
	Token name;
	int depth;
} Local;

//parser structure
typedef struct {
	//reference the scanner
	Scanner* scanner;

	//process the chunk
	Chunk* chunk;
	Token current;
	Token previous;

	//handle local vars/consts
	Local* locals;
	int localCapacity;
	int localCount;
	int scopeDepth;

	//error handling
	bool hadError;
	bool panicMode;
} Parser;

//parser rules
typedef void (*ParseFn)(Parser* parser, bool canAssign);

typedef enum { //TODO: add Toy's extra precedences
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} Precedence;

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	//TODO: mixfix for ternary operator?
	Precedence precedence;
} ParseRule;

ParseRule* getRule(TokenType type);

//initialize the parser
void initParser(Parser* parser, Scanner* scanner, Chunk* chunk);
void freeParser(Parser* parser);

//compiler.c utility functions
void errorAt(Parser* parser, Token* token, const char* message);
void errorAtPrevious(Parser* parser, const char* message);
void errorAtCurrent(Parser* parser, const char* message);

void advance(Parser* parser);
void consume(Parser* parser, TokenType type, const char* message);
bool match(Parser* parser, TokenType type);
bool check(Parser* parser, TokenType type);

//parser.c functions
void emitByte(Parser* parser, uint8_t byte);
void emitBytes(Parser* parser, uint8_t byte1, uint8_t byte2);
uint32_t emitConstant(Parser* parser, Value value);

//grammar rules
void declaration(Parser* parser);
void statement(Parser* parser);
void expression(Parser* parser);

#endif
