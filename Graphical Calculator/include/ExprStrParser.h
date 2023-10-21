#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

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
		std::function<double()> expr;
		double* x_var = new double(0.0f); //cache x separately (also if not pointer, something sometimes goes wrong)
		std::map<std::string, double>* other_vars = new std::map<std::string, double>();
		std::function<double()> calcNodes(const Node* node);
		void calcFunc(const Node* tree);
	public:
		std::function<double()>* GetInternalFunction();
		std::map<std::string, double> GetArgs();
		void SetArgs(const double x);
		void SetArgs(const std::string& name, const double value);
		void SetArgs(const std::map<std::string, double>& args);
		double Calculate();
		double Calculate(const double x);
		double Calculate(const std::string& name, const double value);
		double Calculate(const std::map<std::string, double>& args);
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
		void Parse(std::string& str);

		Expression CopyExpression();

		std::map<std::string, double> GetArgs();
		void SetArgs(const double x);
		void SetArgs(const std::string& name, const double value);
		void SetArgs(const std::map<std::string, double>& args);
		double Calculate();
		double Calculate(const double x);
		double Calculate(const std::string& name, const double value);
		double Calculate(const std::map<std::string, double>& args);
	};

}