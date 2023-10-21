#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>

#define ADD_OPTIONAL_TOKENS

namespace ExprStrParser {

#define pi_str "3.14159265358979323846"
#define e_str  "2.71828182845904523536"
	const std::map<std::string, std::string> math_consts{ {"pi", pi_str}, {"e", e_str} };
	const std::set<std::string> math_functions{ "log", "logn", "sin", "cos", "tan", "arcsin", "arccos", "arctan","sqrt", "ceil", "floor", "round", "abs" , "mod" };

	struct Token {
		enum Type {
			NUL = 0,
			EndOfFile,
			Number, //previoulsy NUM
			Identifier, //previoulsy STR
			Function, //previoulsy COP
			Comma,
			Equals,
			Plus,
			Minus,
			Star,
			Slash,
			Caret,
			ExclamationMark,
			OpenParenthesis,
			CloseParenthesis,
		} type;
		std::string_view val;
		Token():type(NUL), val() {}
		Token(const Type _type,const std::string_view _val): type(_type), val(_val){}
		inline bool operator==(const Token& a) const { return a.type == type && a.val == val; }
		inline bool operator!=(const Token& a) const { return !(a.type == type && a.val == val); }
		inline friend std::ostream& operator<<(std::ostream& os, const Token& tk) {
			os << std::string(tk.val);
			return os;
		}
	};

	class Tokenizer {
	private:
		char* start = nullptr;
		char* current = nullptr;
		inline char advance();
		inline char peek();
		inline void skipSpace();
		Token nextToken();
		Token makeNumber();
		Token makeIdentifier();
		Token makeToken(Token::Type type);
	public:
		void Tokenize(const std::string& input);
		std::vector<Token> tokens;
	};
}
