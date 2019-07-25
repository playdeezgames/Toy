namespace Toy {
	public enum TokenType {
		//single character tokens
		LEFT_PAREN, RIGHT_PAREN,
		LEFT_BRACE, RIGHT_BRACE,
		LEFT_BRACKET, RIGHT_BRACKET,
		SEMICOLON, COMMA,
		SLASH, STAR,
		MODULO,

		//one or two character tokens
		PLUS,
		PLUS_EQUAL,
		PLUS_PLUS,
		MINUS,
		MINUS_EQUAL,
		MINUS_MINUS,

		BANG, BANG_EQUAL,
		EQUAL, EQUAL_EQUAL, EQUAL_GREATER, //EQUAL_GREATER is for the arrow syntax
		GREATER, GREATER_EQUAL,
		LESS, LESS_EQUAL,

		//these can ONLY be doubles
		AND_AND,
		OR_OR,

		//these can single OR triple
		DOT, DOT_DOT_DOT,

		//ternary operator
		QUESTION, COLON,

		//literals
		IDENTIFIER, STRING, NUMBER,

		//keywords (nil = null)
		NIL, PRINT, IMPORT, VAR, CONST, TRUE, FALSE, FUNCTION,
		IF, ELSE, DO, WHILE, FOR, FOREACH, IN, OF,

		EOF
	}
}