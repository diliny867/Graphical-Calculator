#include "../include/ExprStrParser.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>

namespace ExprStrParser { //TODO: Fix comma

#define str_compare(s1, s2) (s1.compare(s2) == 0)

	void Node::print(const std::string& prefix, const bool isLeft) const {
		std::cout << prefix;
		std::cout << (isLeft ? "|-- " : "L-- ");
		//std::cout << this << std::endl;
		std::cout << value << std::endl;

		if (node_count>0) {
			nodes[0]->print(prefix + (isLeft ? "|   " : "    "), true);
			for(std::size_t i=1;i<node_count;i++) {
				nodes[i]->print(prefix + (isLeft ? "|   " : "    "), false);
			}
		}
	}

	std::function<double()>* Expression::GetInternalFunction() {
		return &expr;
	}

	std::function<double()> Expression::calcNodes(const Node* node) {
		if(node == nullptr) { return []() { return std::numeric_limits<double>::quiet_NaN();}; }
		const Token curr_token = node->value;

		const auto calculated_nodes = new std::function<double()>[node->node_count];

		for(std::size_t i=0;i<node->node_count;i++) {
			calculated_nodes[i] = calcNodes(node->nodes[i]);
		}
		
		switch (curr_token.type) {
		case Token::Number: 
		{
			const double num = std::stod(std::string(curr_token.val));
			return [=]() {return num; };
		}
		case Token::Identifier:
		{
			if(curr_token.val == "x") {
				return[=]() {
					return *x_var;
				};
			} else {
				const std::string func_name = std::string(curr_token.val);
				other_vars->try_emplace(func_name, 0.0);
				return [=]() {return (*other_vars)[func_name]; };
			}
		}
		case Token::Plus:
			return [=]() {return calculated_nodes[0]()+calculated_nodes[1](); };
		case Token::Minus:
			return [=]() {return calculated_nodes[0]()-calculated_nodes[1](); };
		case Token::Star:
			return [=]() {return calculated_nodes[0]()*calculated_nodes[1](); };
		case Token::Slash:
			return [=]() {return calculated_nodes[0]()/calculated_nodes[1](); };
		case Token::Caret:
			return [=]() {return std::pow(calculated_nodes[0](), calculated_nodes[1]()); };
		case Token::ExclamationMark:
			return [=]() {return std::tgamma(calculated_nodes[0]()+1.0); };
		case Token::Function:
			if (str_compare(curr_token.val,"log")) {
				return [=]() {return std::log(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "sin")) {
				return [=]() {return std::sin(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "cos")) {
				return [=]() {return std::cos(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "tan")) {
				return [=]() {return std::tan(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "arcsin")) {
				return [=]() {return std::asin(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "arccos")) {
				return [=]() {return std::acos(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "arctan")) {
				return [=]() {return std::atan(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "sqrt")) {
				return [=]() {return std::sqrt(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "ceil")) {
				return [=]() {return std::ceil(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "floor")) {
				return [=]() {return std::floor(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "round")) {
				return [=]() {return std::round(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "abs")) {
				return [=]() {return std::abs(calculated_nodes[0]()); };
			}
			if (str_compare(curr_token.val, "mod")) {
				if(node->node_count<2) {
					return []() { return std::numeric_limits<double>::quiet_NaN();};
				}
				return [=]() {return std::fmod(calculated_nodes[0](), calculated_nodes[1]()); };
			}
			if (str_compare(curr_token.val, "logn")) {
				if(node->node_count<2) {
					return []() { return std::numeric_limits<double>::quiet_NaN();};
				}
				return [=]() {return std::log(calculated_nodes[0]())/std::log(calculated_nodes[1]()); };
			}
			break;
		default:
			return []() {return 0.0; };
		}
		return []() {return 0.0; };
	}

	bool Parser::buildTokenTree() {
		try {
			tree = rcalcNode(tokenizer.tokens.crbegin(), tokenizer.tokens.crend());
			tokenizer.tokens.clear();
			return true;
		} catch (std::exception& e) {
			tree = nullptr;
			tokenizer.tokens.clear();
			return false;
		}
	}

	Node* Parser::rcalcNode(const std::vector<Token>::const_reverse_iterator& rit_begin, const std::vector<Token>::const_reverse_iterator& rit_end) {
		if (rit_end-rit_begin <= 0) {
			throw std::exception("PARSE ERROR");	
		}
		if (rit_end-rit_begin == 1) { //leaf
			if(rit_begin->type == Token::Number || rit_begin->type == Token::Identifier) {
				return new Node(*rit_begin);
			}
			throw std::exception("PARSE ERROR");	
		}

		{
			auto rit_tmp_begin = rit_begin;
			auto rit_tmp_end = rit_end-1;
			int count = 0;
			while(rit_tmp_begin->type == Token::CloseParenthesis && rit_tmp_end->type == Token::OpenParenthesis) {
				++rit_tmp_begin;
				--rit_tmp_end;
				count++;
			}
			if(count) {
				return rcalcNode(rit_begin + count, rit_end - count);
			}
		}

		Node* curr_node;

		if ((rit_end-1)->type == Token::Function) { //process COP
			if (rit_begin->type == Token::CloseParenthesis && (rit_end-2)->type == Token::OpenParenthesis) {
				int level = 0;
				for (auto rit = rit_begin+1; rit<rit_end-2; ++rit) {
					if (rit->type == Token::OpenParenthesis){
						--level;
						if (level < 0) {
							break;
						}
					}else
					if (rit->type == Token::CloseParenthesis) {
						++level;
					}else
					if (rit->type == Token::Comma && level == 0) {
						curr_node = new Node(*(rit_end-1));
						curr_node->node_count = 2;
						curr_node->nodes = new Node*[2];
						curr_node->nodes[0] = rcalcNode(rit+1, rit_end-2);
						curr_node->nodes[1] = rcalcNode(rit_begin+1, rit);
						return curr_node;
					}
				}
				if (level == 0) {
					curr_node = new Node(*(rit_end-1));
					curr_node->node_count = 1;
					curr_node->nodes = new Node*[1];
					curr_node->nodes[0] = rcalcNode(rit_begin, rit_end-1);
					return curr_node;
				}
			}
		}

		int level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {
			switch(rit->type) {
			case Token::CloseParenthesis:
				++level;
				break;
			case Token::OpenParenthesis:
				--level;
				break;
			case Token::Minus:
			case Token::Plus:
				if (level == 0) {
					curr_node = new Node(Token(*rit));
					curr_node->node_count = 2;
					curr_node->nodes = new Node*[2];
					curr_node->nodes[0] = rcalcNode(rit+1, rit_end);
					curr_node->nodes[1] = rcalcNode(rit_begin, rit);
					return curr_node;
				}
				break;
			}
		}

		level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {
			switch(rit->type) {
			case Token::CloseParenthesis:
				++level;
				break;
			case Token::OpenParenthesis:
				--level;
				break;
			case Token::Star:
			case Token::Slash:
				if (level == 0) {
					curr_node = new Node(*rit);
					curr_node->node_count = 2;
					curr_node->nodes = new Node*[2];
					curr_node->nodes[0] = rcalcNode(rit+1, rit_end);
					curr_node->nodes[1] = rcalcNode(rit_begin, rit);
					return curr_node;
				}
				break;
			}
		}

		level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {// ^ and ! need to be processed separately, because they should affect only the closest token(s)
			switch(rit->type) {
			case Token::CloseParenthesis:
				++level;
				break;
			case Token::OpenParenthesis:
				--level;
				break;
			case Token::Caret:
				if (level == 0) {
					curr_node = new Node(*rit);
					curr_node->node_count = 2;
					curr_node->nodes = new Node*[2];
					curr_node->nodes[0] = rcalcNode(rit+1, rit_end);
					curr_node->nodes[1] = rcalcNode(rit_begin, rit);
					return curr_node;
				}
				break;
			case Token::ExclamationMark:
				if (level == 0) {
					curr_node = new Node(*rit);
					curr_node->node_count = 1;
					curr_node->nodes = new Node*[1];
					curr_node->nodes[0] = rcalcNode(rit+1, rit_end);
					return curr_node;
				}
				break;
			}
		}

		throw std::exception("PARSE ERROR");	
	}

	void Parser::Parse(std::string& str) {
		tokenizer.Tokenize(str);
		curr_expression.other_vars->clear();
		if (buildTokenTree()) {
			curr_expression.CalcFunc(tree);
		}else {
			curr_expression.expr = []() {return std::numeric_limits<double>::quiet_NaN(); };
		}
		curr_expression.tree = tree;
		//if(tree != nullptr){
		//	tree->print("", false);
		//}
	}

	Expression Parser::CopyExpression() {
		return curr_expression.Copy();
	}

	Expression::Expression() {
		other_vars = new std::unordered_map<std::string, double>();
		x_var = &(*other_vars)["x"];
	}
	Expression::Expression(const std::unordered_map<std::string, double>* args) {
		other_vars = new std::unordered_map<std::string, double>(*args);
		x_var = &(*other_vars)["x"];
	}
	Expression Expression::Copy() {
		Expression expression(other_vars);
		expression.tree = tree;
		expression.CalcFunc(tree);
		return expression;
	}

	void Expression::CalcFunc(Node* tree_) {
		tree = tree_;
		expr = calcNodes(tree);
	}

	double Expression::GetArg(const std::string& name) {
		if(name == "x") {
			return *x_var;
		}
		return (*other_vars)[name];
	}
	std::unordered_map<std::string, double> Expression::GetArgs() {
		return *other_vars;
	}
	void Expression::SetArgs(const double x) {
		*x_var = x;
	}
	void Expression::SetArgs(const std::string& name, const double value) {
		(*other_vars)[name] = value;
	}
	void Expression::SetArgs(const std::unordered_map<std::string, double>& args) {
		*other_vars = args;
	}
	double Expression::Calculate() {
		return expr();
	}
	double Expression::Calculate(const double x) {
		*x_var = x;
		return expr();
	}
	double Expression::Calculate(const std::string& name, const double value) {
		(*other_vars)[name] = value;
		return expr();
	}
	double Expression::Calculate(const std::unordered_map<std::string, double>& args) {
		*other_vars = args;
		return expr();
	}

	std::unordered_map<std::string, double> Parser::GetArgs() {
		return curr_expression.GetArgs();
	}
	void Parser::SetArgs(const double x) {
		curr_expression.SetArgs(x);
	}
	void Parser::SetArgs(const std::string& name, const double value) {
		curr_expression.SetArgs(name, value);
	}
	void Parser::SetArgs(const std::unordered_map<std::string, double>& args) {
		curr_expression.SetArgs(args);
	}
	double Parser::Calculate() {
		return curr_expression.Calculate();
	}
	double Parser::Calculate(const double x) {
		return curr_expression.Calculate(x);
	}
	double Parser::Calculate(const std::string& name, const double value) {
		return curr_expression.Calculate(name,value);
	}
	double Parser::Calculate(const std::unordered_map<std::string, double>& args) {
		return curr_expression.Calculate(args);
	}
}
