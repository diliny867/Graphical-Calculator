#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <functional>
#include <map>
#include <set>
#include <cstring>
#include <cmath>
//#include <chrono>

namespace ExprStrParser {

	enum symbol {
		NUL = 0,
		NUM,//number
		OP,//operator (+, -, *...)
		COP,//complex operator (like log or sin)
		STR//string (includes for example x and so on)
	};

	enum error_codes {
		PARSE_ERROR,
		SOLVE_ERROR
	};

	struct token {
		token():val(), symb(NUL) {}
		token(const std::string& _val, const symbol _symb):val(_val), symb(_symb) {}
		std::string val;
		symbol symb;
		bool operator==(const token& a) const { return a.symb == symb && a.val == val; }
		bool operator!=(const token& a) const { return !(a.symb == symb && a.val == val); }
		friend std::ostream& operator<<(std::ostream& os, const token& tk);
	};
	inline std::ostream& operator<<(std::ostream& os, const token& tk) {
		os << tk.val;
		//os << tk.val <<": " << tk.symb;
		return os;
	}

	class Node {
	public:
		Node() = default;
		Node(const token& _val):value(_val) {}
		token value;
		Node* left = nullptr;
		Node* right = nullptr;
		void print(const std::string& prefix, const bool isLeft) const;
	};
	class Tree {
	public:
		Tree() { head = new Node(); }
		void print() const;
		Node* head;
	};

	class Expression {
	public:
		Expression() = default;
		std::function<float()> expr;
		std::map<std::string, float> func_args;
		float x = 0.0f;//cache x, to call it and assign faster
		std::function<float()> calc_nodes(const Node* node);
		//float calc_nodes(const Node* node);
		void calc_func(const Tree* tree);
	};

	class Parser {
	private:
		static inline std::set<std::string> cop_set{ "log", "sin", "cos", "tan", "sqrt", "ceil", "floor", "round", "abs" , "mod" };
		Tree tree;
		Expression expression{};
		Node* rcalcNode(const std::vector<token>::reverse_iterator& rit_begin, const std::vector<token>::reverse_iterator& rit_end);
		bool buildTokenTree();
		std::vector<token> tokens;
		void check_str_sstream(std::stringstream& str_ss);
		void check_num_sstream(std::stringstream& num_ss);
		void tokenize(std::string& str);
		void set_func();
		//void optizmieTree();//maybe could later create this (to for example calculate multiple multiplications at one, first time) (or maybe add this method to expr tree?)
	public:
		Parser() = default;
		void parse(std::string& str);
		std::map<std::string, float> get_args();
		void set_args(const float x);
		void set_args(const std::string& name, const float value);
		void set_args(const std::pair<std::string, float>& arg);
		void set_args(const std::map<std::string, float>& args);
		void set_args(const float x, const std::map<std::string, float>& args);
		float calculate()const;
		float calculate(const float x);
		float calculate(const std::string& name, const float value);
		float calculate(const std::pair<std::string, float>& arg);
		float calculate(const std::map<std::string, float>& args);
		float calculate(const float x, const std::map<std::string, float>& args);
	};

}