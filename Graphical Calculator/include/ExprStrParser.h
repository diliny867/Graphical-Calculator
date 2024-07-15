#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "ExprStrTokenizer.h"

namespace ExprStrParser {

	class Node {
	public:
		Node():value(Token()) {};
		Node(const Token& _val):value(_val) {}
		Token value;
		std::size_t node_count = 0;
		Node** nodes;
		void print(const std::string& prefix, const bool isLeft) const;
	};

	class Expression {
	private:
		friend class Parser;
		Node* tree = nullptr;
		std::function<double()> expr;
		std::unordered_map<std::string, double>* other_vars;
		double* x_var; //cache ptr to x separately
		std::function<double()> calcNodes(const Node* node);
	public:
		Expression();
		Expression(const std::unordered_map<std::string, double>* args);
		Expression Copy();
		void CalcFunc(Node* tree_);

		std::function<double()>* GetInternalFunction();
		double GetArg(const std::string& name);
		std::unordered_map<std::string, double> GetArgs();
		inline void SetArgs(const double x);
		inline void SetArgs(const std::string& name, const double value);
		inline void SetArgs(const std::unordered_map<std::string, double>& args);
		inline double Calculate();
		inline double Calculate(const double x);
		inline double Calculate(const std::string& name, const double value);
		inline double Calculate(const std::unordered_map<std::string, double>& args);
	};

	class Parser {
	private:
		Tokenizer tokenizer;
		Node* tree;
		Expression curr_expression;
		Node* rcalcNode(const std::vector<Token>::const_reverse_iterator& rit_begin, const std::vector<Token>::const_reverse_iterator& rit_end);
		bool buildTokenTree();
	public:
		Parser() = default;
		void Parse(const std::string& str);
		Expression CopyExpression();

		std::unordered_map<std::string, double> GetArgs();
		void SetArgs(const double x);
		void SetArgs(const std::string& name, const double value);
		void SetArgs(const std::unordered_map<std::string, double>& args);
		double Calculate();
		double Calculate(const double x);
		double Calculate(const std::string& name, const double value);
		double Calculate(const std::unordered_map<std::string, double>& args);
	};

}