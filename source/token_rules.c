#include "token_rules.h"
#include "parser_tools.h"
#include "grammar_rules.h"

#include <stdlib.h>

//precedence
void parsePrecendence(Parser* parser, Precedence precedence) {
	advance(parser);

	ParseFn prefixRule = getRule(parser->previous.type)->prefix;
	if (prefixRule == NULL) {
		errorAtCurrent(parser, "Expected expression");
		return;
	}

	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(parser, canAssign);

	while (precedence <= getRule(parser->current.type)->precedence) {
		advance(parser);
		ParseFn infixRule = getRule(parser->previous.type)->infix;
		infixRule(parser, canAssign);
	}

	if (canAssign && match(parser, TOKEN_EQUAL)) {
		errorAtPrevious(parser, "Invalid assignment target");
	}
}

//expressions
static void variable(Parser* parser, bool canAssign) {
	namedVariable(parser, parser->previous, canAssign);
}

static void number(Parser* parser, bool canAssign) {
	double value = strtod(parser->previous.start, NULL);
	emitConstant(parser, NUMBER_VAL(value));
}

static void string(Parser* parser, bool canAssign) {
	//TODO: escape characters
	emitConstant(parser, OBJECT_VAL(copyString(&parser->chunk->objects, &parser->chunk->strings, parser->previous.start + 1, parser->previous.length - 2)));
}

static void grouping(Parser* parser, bool canAssign) {
	expression(parser);
	consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
}

static void unary(Parser* parser, bool canAssign) {
	TokenType operatorType = parser->previous.type;

	parsePrecendence(parser, PREC_UNARY);

	switch(operatorType) {
		case TOKEN_MINUS:
			emitByte(parser, OP_NEGATE);
			break;

		case TOKEN_BANG:
			emitByte(parser, OP_NOT);
			break;

		default:
			return;
	}
}

static void binary(Parser* parser, bool canAssign) { //TODO: can I compute literals in the parser?
	TokenType operatorType = parser->previous.type;

	ParseRule* rule = getRule(operatorType);
	parsePrecendence(parser, (Precedence)(rule->precedence + 1));

	switch(operatorType) {
		//comparisons
		case TOKEN_EQUAL_EQUAL:
			emitByte(parser, OP_EQUAL);
			break;

		case TOKEN_BANG_EQUAL:
			emitBytes(parser, OP_EQUAL, OP_NOT);
			break;

		case TOKEN_GREATER:
			emitByte(parser, OP_GREATER);
			break;

		case TOKEN_GREATER_EQUAL:
			emitBytes(parser, OP_LESS, OP_NOT);
			break;

		case TOKEN_LESS:
			emitByte(parser, OP_LESS);
			break;

		case TOKEN_LESS_EQUAL:
			emitBytes(parser, OP_GREATER, OP_NOT);
			break;

		//arithmetic
		case TOKEN_PLUS:
			emitByte(parser, OP_ADD);
			break;

		case TOKEN_MINUS:
			emitByte(parser, OP_SUBTRACT);
			break;

		case TOKEN_STAR:
			emitByte(parser, OP_MULTIPLY);
			break;

		case TOKEN_SLASH:
			emitByte(parser, OP_DIVIDE);
			break;

		default:
			return;
	}
}

static void atom(Parser* parser, bool canAssign) {
	switch(parser->previous.type) {
		case TOKEN_FALSE: emitByte(parser, OP_FALSE); break;
		case TOKEN_NIL: emitByte(parser, OP_NIL); break;
		case TOKEN_TRUE: emitByte(parser, OP_TRUE); break;
		default:
			return;
	}
}

//a pratt table
ParseRule parseRules[] = {
	//single character tokens
	{grouping,		NULL,			PREC_NONE}, // TOKEN_LEFT_PAREN,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_RIGHT_PAREN,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_LEFT_BRACE,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_RIGHT_BRACE,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_LEFT_BRACKET,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_RIGHT_BRACKET,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_SEMICOLON,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_COMMA,

	//one or two character tokens
	{NULL,			binary,			PREC_TERM}, // TOKEN_PLUS,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_PLUS_EQUAL,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_PLUS_PLUS,

	{unary,			binary,			PREC_TERM}, // TOKEN_MINUS,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_MINUS_EQUAL,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_MINUS_MINUS,
	//TODO: minus greater for objects

	{NULL,			binary,			PREC_FACTOR}, // TOKEN_STAR,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_STAR_EQUAL,

	{NULL,			binary,			PREC_FACTOR}, // TOKEN_SLASH,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_SLASH_EQUAL,

	{NULL,			NULL,			PREC_NONE}, // TOKEN_MODULO,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_MODULO_EQUAL,

	{unary,			NULL,			PREC_NONE}, // TOKEN_BANG,
	{NULL,			binary,			PREC_EQUALITY}, // TOKEN_BANG_EQUAL,

	{NULL,			NULL,			PREC_NONE}, // TOKEN_EQUAL,
	{NULL,			binary,			PREC_EQUALITY}, // TOKEN_EQUAL_EQUAL,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_EQUAL_GREATER, //EQUAL_GREATER is for the arrow syntax

	{NULL,			binary,			PREC_COMPARISON}, // TOKEN_GREATER,
	{NULL,			binary,			PREC_COMPARISON}, // TOKEN_GREATER_EQUAL,

	{NULL,			binary,			PREC_COMPARISON}, // TOKEN_LESS,
	{NULL,			binary,			PREC_COMPARISON}, // TOKEN_LESS_EQUAL,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_LESS_OR, //back pipe

	//these can ONLY be doubles
	{NULL,			NULL,			PREC_NONE}, // TOKEN_AND_AND,

	{NULL,			NULL,			PREC_NONE}, // TOKEN_OR_OR,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_OR_GREATER, //pipe

	//these can single OR triple
	{NULL,			NULL,			PREC_NONE}, // TOKEN_DOT,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_DOT_DOT_DOT, //reserved for the rest/spread operator

	//ternary operator
	{NULL,			NULL,			PREC_NONE}, // TOKEN_QUESTION,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_COLON,

	//literals
	{variable,		NULL,			PREC_NONE}, // TOKEN_IDENTIFIER,
	{number,		NULL,			PREC_NONE}, // TOKEN_NUMBER,
	{string,		NULL,			PREC_NONE}, // TOKEN_STRING,

	//keywords (alphabetized)
	{NULL,			NULL,			PREC_NONE}, // TOKEN_AS,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_ASSERT,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_BREAK,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_CONST,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_CONTINUE,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_DO,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_ELSE,
	{atom,			NULL,			PREC_NONE}, // TOKEN_FALSE,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_FOR,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_FOREACH, //reserved
	{NULL,			NULL,			PREC_NONE}, // TOKEN_IF,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_IMPORT,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_IN, //reserved
	{atom,			NULL,			PREC_NONE}, // TOKEN_NIL, //null
	{NULL,			NULL,			PREC_NONE}, // TOKEN_OF, //reserved
	{NULL,			NULL,			PREC_NONE}, // TOKEN_PRINT,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_RETURN,
	{atom,			NULL,			PREC_NONE}, // TOKEN_TRUE,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_VAR,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_WHILE,

	//meta
	{NULL,			NULL,			PREC_NONE}, // TOKEN_PASS, //do nothing
	{NULL,			NULL,			PREC_NONE}, // TOKEN_ERROR,
	{NULL,			NULL,			PREC_NONE}, // TOKEN_EOF
};

ParseRule* getRule(TokenType type) {
	return &parseRules[type];
}