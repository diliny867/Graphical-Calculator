#include "../include/ExprStrTokenizer.h"

#include <string>
#include <iostream>


namespace ExprStrParser {

	void Tokenizer::Tokenize(const std::string& input) {
		if(input.empty()) {
			return;
		}
		current = const_cast<char*>(input.data());
		start = current;

		for(Token token = nextToken(); token.type != Token::Type::EndOfFile; token = nextToken()) {
			tokens.push_back(token);
		}
	}

	inline char Tokenizer::advance() {
		return *(current++);
	}
	inline char Tokenizer::peek() {
		return *current;
	}
	inline void Tokenizer::skipSpace() {
		while(isspace(peek())) {
			advance();
		}
	}

	Token Tokenizer::makeToken(const Token::Type type) {
		return Token(type,std::string_view(start,current-start));
	}
	Token Tokenizer::makeNumber() {
		while(isdigit(peek())) {
			advance();
		}
		if(peek() == '.') {
			advance();
			while(isdigit(peek())) {
				advance();
			}
		}
		return makeToken(Token::Type::Number);
	}
	Token Tokenizer::makeIdentifier() {
		while(isdigit(peek()) || isalpha(peek())) {
			advance();
		}
		Token token = makeToken(Token::Type::Identifier);
		std::string token_str = std::string(token.val);
		if(math_consts.count(token_str)) {
			token.val = math_consts.at(token_str);
			token.type = Token::Number;
		}else 
		if(math_functions.count(token_str) ) {
			token.type = Token::Function;
		}
		return token;
	}

	Token Tokenizer::nextToken() {
		skipSpace();
		start = current;
		if(peek() == '\0') {
			return makeToken(Token::Type::EndOfFile);
		}

#ifdef ADD_OPTIONAL_TOKENS
		const Token::Type prev = tokens.empty() ? Token::NUL : tokens.back().type;
#endif

		const char c = advance();
		switch(c) {
		case '(':
#ifdef ADD_OPTIONAL_TOKENS
			if(prev == Token::CloseParenthesis || prev == Token::Number || prev == Token::Identifier){
				tokens.push_back(Token(Token::Star, "*"));
			}
#endif
			return makeToken(Token::Type::OpenParenthesis);
		case ')':
			return makeToken(Token::Type::CloseParenthesis);
		case ',':
			return makeToken(Token::Type::Comma);
		case '=':
			return makeToken(Token::Type::Equals);
		case '+':
			return makeToken(Token::Type::Plus);
		case '-':
#ifdef ADD_OPTIONAL_TOKENS
			if(prev == Token::OpenParenthesis || tokens.empty()) {
				tokens.push_back(Token(Token::Number, "0"));
			}
#endif
			return makeToken(Token::Type::Minus);
		case '*':
			return makeToken(Token::Type::Star);
		case '/':
			return makeToken(Token::Type::Slash);
		case '^':
			return makeToken(Token::Type::Caret);
		case '!':
			return makeToken(Token::Type::ExclamationMark);
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
#ifdef ADD_OPTIONAL_TOKENS
			if(prev == Token::Number || prev == Token::Identifier){
				tokens.push_back(Token(Token::Star, "*"));
			}
#endif
			return makeNumber();
		default:
			if(isalpha(c)) {
#ifdef ADD_OPTIONAL_TOKENS
				if(prev == Token::Number || prev == Token::Identifier){
					tokens.push_back(Token(Token::Star, "*"));
				}
#endif
				return makeIdentifier();
			}
		}
		return makeToken(Token::Type::NUL);
	}


}