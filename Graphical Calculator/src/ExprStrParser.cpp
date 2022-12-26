#include "../include/ExprStrParser.h"

namespace ExprStrParser {

#define pi_str "3.14159265358979323846"
#define e_str  "2.71828182845904523536"
	const std::map<std::string, std::string> math_consts{ {"pi", pi_str}, {"e", e_str} };

#define str_compare(s1, s2) (std::strcmp(s1, s2) == 0)

	void Node::print(const std::string& prefix, const bool isLeft) const {
		std::cout << prefix;
		std::cout << (isLeft ? "|-- " : "L-- ");
		//std::cout << this << std::endl;
		std::cout << value << std::endl;

		if (left!=nullptr) {
			left->print(prefix + (isLeft ? "|   " : "    "), true);
		}
		if (right!=nullptr) {
			right->print(prefix + (isLeft ? "|   " : "    "), false);
		}
	}

	void Tree::print() const {
		head->print("", false);
	}


	void Expression::calc_func(const Tree* tree) {
		try {
			expr = calc_nodes(tree->head);
		} catch (error_codes err) {
			expr = []() {return std::nanf(""); };
		}
	}

	std::function<float()> Expression::calc_nodes(const Node* node) {
		if (node == nullptr) { throw SOLVE_ERROR; }
		const token curr_token = node->value;

		switch (curr_token.symb) {
		case NUM://NUM and STR cover the case when both left and right nodes are nullptr
			return [=]() {return std::stof(curr_token.val); };
		case STR:
			if (curr_token.val == "x") {
				return[=]() {return x; };
			} else {
				return [=]() {return func_args[curr_token.val]; };
			}
		case OP:
		{
			const auto left = calc_nodes(node->left);
			if (str_compare(curr_token.val.c_str(), "!")) {
				return [=]() {return std::tgamma(left()+1.0f); };
			}
			const auto right = calc_nodes(node->right);
			if (str_compare(curr_token.val.c_str(), "+")) {
				return [=]() {return left()+right(); };
			}
			if (str_compare(curr_token.val.c_str(), "-")) {
				return [=]() {return left()-right(); };
			}
			if (str_compare(curr_token.val.c_str(), "*")) {
				return [=]() {return left()*right(); };
			}
			if (str_compare(curr_token.val.c_str(), "/")) {
				return [=]() {return left()/right(); };
			}
			if (str_compare(curr_token.val.c_str(), "^")) {
				return [=]() {return pow(left(), right()); };
			}
		}
		break;
		case COP:
		{
			const auto right = calc_nodes(node->right);
			if (str_compare(curr_token.val.c_str(), "log")) {
				return [=]() {return std::log(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "sin")) {
				return [=]() {return sin(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "cos")) {
				return [=]() {return cos(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "tan")) {
				return [=]() {return tan(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "sqrt")) {
				return [=]() {return sqrt(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "ceil")) {
				return [=]() {return ceil(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "floor")) {
				return [=]() {return floor(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "round")) {
				return [=]() {return round(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "abs")) {
				return [=]() {return abs(right()); };
			}
			if (str_compare(curr_token.val.c_str(), "mod")) {
				const auto left = calc_nodes(node->left);
				return [=]() {return std::fmod(left(), right()); };
			}
		}
		break;
		case NUL:
			return []() {return 0.0f; };
		}
		return []() {return 0.0f; };
	}

	void Parser::check_str_sstream(std::stringstream& str_ss) {// !TODO: parse things like xlog to x and log, not x l o g
		if (!str_ss.str().empty()) {
			std::string str_;
			str_ss>>str_;
			if (!tokens.empty()) {
				if (tokens.back().symb == NUM || tokens.back().symb == STR || str_compare(tokens.back().val.c_str(), ")")) {
					tokens.push_back(token("*", OP));
				}
			}
			if (cop_set.count(str_)) {
				tokens.push_back(token(str_, COP));
			} else if (math_consts.count(str_)) {
				tokens.push_back(token(math_consts.at(str_), NUM));
			} else {
				for (const auto& ch : str_) { //converts abc to a*b*c
					tokens.push_back(token(std::string(1, ch), STR));
					if (tokens.back().val == "x") {
						expression.x = 0.0f;
					} else {
						expression.func_args[tokens.back().val] = 0.0f;
					}
					tokens.push_back(token("*", OP));
				}
				tokens.pop_back();
				//tokens.push_back(token(str_, STR));
			}
			str_ss = std::stringstream();
		}
	}
	void Parser::check_num_sstream(std::stringstream& num_ss) {
		if (!num_ss.str().empty()) {
			tokens.push_back(token(num_ss.str(), NUM));
			num_ss = std::stringstream();
		}
	}

	void Parser::tokenize(std::string& str) {
		tokens.reserve(str.size());
		std::stringstream num_ss;
		std::stringstream str_ss;

		for (auto it = str.begin(); it != str.end(); ++it) {
			if (*it==' ') { continue; }
			if (isdigit(*it) || *it == '.') {
				num_ss<<*it;
				check_str_sstream(str_ss);
			} else {
				check_num_sstream(num_ss);
				if (ispunct(*it)) {
					check_str_sstream(str_ss);
					if (*it == '-') {
						if (tokens.empty() || str_compare(tokens.back().val.c_str(), "(")) {
							tokens.push_back(token("0", NUM));
						}
					}
					if (*it == '(') {
						if (!tokens.empty()) {
							if (tokens.back().symb == NUM || tokens.back().symb == STR || str_compare(tokens.back().val.c_str(), ")")) {
								tokens.push_back(token("*", OP));
							}
						}
					}
					tokens.push_back(token(std::string(1, *it), OP));
				} else {
					str_ss<<*it;
					if (math_consts.count(str_ss.str())) {
						if (!tokens.empty()) {
							if (tokens.back().symb == NUM || tokens.back().symb == STR || str_compare(tokens.back().val.c_str(), ")")) {
								tokens.push_back(token("*", OP));
							}
						}
						tokens.push_back(token(math_consts.at(str_ss.str()), NUM));
						str_ss = std::stringstream();
					}
					//else {
						//std::string str_ = str_ss.str();
						//
						//for (auto rit = str_.rbegin(); rit<str_.rend(); ++rit) {
						//	
						//	if (cop_set.count(std::string(rit.base(), str_.rbegin().base()))) {
						//		//const token new_cop_token = token(std::string(rit.base(), str_.rbegin().base()), COP);
						//		for(auto _it = str_.begin(); _it<rit.base(); ++_it) {
						//			//std::cout<<*_it;
						//			tokens.push_back(token(std::string(1, *_it), STR));
						//			tokens.push_back(token("*", OP));
						//		}
						//		//tokens.push_back(new_cop_token);
						//		str_ss = std::stringstream();
						//		break;
						//	}
						//}
					//}
				}
			}
		}
		check_num_sstream(num_ss);
		check_str_sstream(str_ss);
	}

	bool Parser::buildTokenTree() {
		try {
			tree.head = rcalcNode(tokens.rbegin(), tokens.rend());
			tokens.clear();
			return true;
		} catch (error_codes err) {
			tokens.clear();
			return false;
			//std::cout<<"ERROR: PARSE ERROR"<<std::endl;
		}
	}

	Node* Parser::rcalcNode(const std::vector<token>::reverse_iterator& rit_begin, const std::vector<token>::reverse_iterator& rit_end) {
		if (rit_end-rit_begin <= 0) {
			throw PARSE_ERROR;
		}
		if (rit_end-rit_begin == 1) {//leaf (symb == NUM or STR)
			return new Node(*rit_begin);
		}

		if (str_compare(rit_begin->val.c_str(), ")") && str_compare((rit_end-1)->val.c_str(), "(")) {//strips expression of side brackets
			int level = 0;
			for (auto rit = rit_begin+1; rit<rit_end-1; ++rit) {//temporary solution(its slow and needs to be checked in next two loops)
				if (rit->val == "(") {
					--level;
					if (level < 0) {
						break;
					}
					continue;
				}
				if (rit->val == ")") {
					++level;
					continue;
				}
			}
			if (level == 0) {
				return rcalcNode(rit_begin + 1, rit_end - 1);
			}
		}

		Node* curr_node;

		if ((rit_end-1)->symb == COP) {//process COP
			if (rit_begin->val == ")" && (rit_end-2)->val == "(") {
				int level = 0;
				for (auto rit = rit_begin+1; rit<rit_end-2; ++rit) {
					if (rit->val == "(") {
						--level;
						if (level < 0) {
							break;
						}
						continue;
					}
					if (rit->val == ")") {
						++level;
						continue;
					}
					if (rit->val == "," && level == 0) {
						curr_node = new Node(*(rit_end-1));
						curr_node->left = rcalcNode(rit+1, rit_end-2);
						curr_node->right = rcalcNode(rit_begin+1, rit);
						return curr_node;
					}
				}
				if (level == 0) {
					curr_node = new Node(*(rit_end-1));
					curr_node->right = rcalcNode(rit_begin, rit_end-1);
					return curr_node;
				}
			}
		}

		int level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {
			if (rit->symb == OP) {
				if (str_compare(rit->val.c_str(), ")")) {
					++level;
					continue;
				}
				if (str_compare(rit->val.c_str(), "(")) {
					--level;
					continue;
				}
				if (level == 0) {
					if (str_compare(rit->val.c_str(), "-") || str_compare(rit->val.c_str(), "+")) {
						curr_node = new Node(token(rit->val, OP));
						curr_node->left = rcalcNode(rit+1, rit_end);
						curr_node->right = rcalcNode(rit_begin, rit);
						return curr_node;
					}
				}
			}
		}

		level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {
			if (rit->symb == OP) {
				if (str_compare(rit->val.c_str(), ")")) {
					++level;
					continue;
				}
				if (str_compare(rit->val.c_str(), "(")) {
					--level;
					continue;
				}
				if (level == 0) {
					if (str_compare(rit->val.c_str(), "^") || str_compare(rit->val.c_str(), "!")) { continue; }
					curr_node = new Node(*rit);
					curr_node->left = rcalcNode(rit+1, rit_end);
					curr_node->right = rcalcNode(rit_begin, rit);
					return curr_node;
				}
			}
		}

		level = 0;
		for (auto rit = rit_begin; rit<rit_end; ++rit) {// ^ and ! need to be processed separately, because they should affect only the closest token(s)
			if (rit->symb == OP) {
				if (str_compare(rit->val.c_str(), ")")) {
					++level;
					continue;
				}
				if (str_compare(rit->val.c_str(), "(")) {
					--level;
					continue;
				}
				if (level == 0) {
					if (str_compare(rit->val.c_str(), "^")) {
						curr_node = new Node(*rit);
						curr_node->left = rcalcNode(rit+1, rit_end);
						curr_node->right = rcalcNode(rit_begin, rit);
						return curr_node;
					}
					if (str_compare(rit->val.c_str(), "!")) {
						curr_node = new Node(*rit);
						curr_node->left = rcalcNode(rit+1, rit_end);
						return curr_node;
					}
				}
			}
		}

		return nullptr;
	}

	void Parser::set_func() {
		expression.calc_func(&tree);
	}

	void Parser::parse(std::string& str) {
		expression.func_args.clear();
		tokenize(str);
		//for (const auto& token: tokens) {
		//	std::cout<< token.val <<" ";
		//	//std::cout << token.val << " " << token.symb << std::endl;
		//}
		//std::cout<<std::endl;
		if (buildTokenTree()) {
			set_func();
		}else {
			expression.expr = []() {return std::nanf(""); };
		}
		//for (auto it = expression.func_args.begin(); it!=expression.func_args.end();++it) {
		//	std::cout<<it->first<<" ";
		//}
		//tree.print();
	}

	std::map<std::string, float> Parser::get_args() {
		return expression.func_args;
	}

	void Parser::set_args(const float x) {
		expression.x = x;
	}
	void Parser::set_args(const std::string& name, const float value) {
		expression.func_args[name] = value;
	}
	void Parser::set_args(const std::pair<std::string, float>& arg) {
		expression.func_args[arg.first] = arg.second;
	}
	void Parser::set_args(const std::map<std::string, float>& args) {
		expression.func_args = args;
	}
	void Parser::set_args(const float x, const std::map<std::string, float>& args) {
		expression.x = x;
		expression.func_args = args;
	}
	float Parser::calculate() const {
		return expression.expr();
	}
	float Parser::calculate(const float x) {
		expression.x = x;
		return expression.expr();
	}
	float Parser::calculate(const std::string& name, const float value) {
		expression.func_args[name] = value;
		return expression.expr();
	}
	float Parser::calculate(const std::pair<std::string, float>& arg) {
		expression.func_args[arg.first] = arg.second;
		return expression.expr();
	}
	float Parser::calculate(const std::map<std::string, float>& args) {
		expression.func_args = args;
		return expression.expr();
	}
	float Parser::calculate(const float x, const std::map<std::string, float>& args) {
		expression.x = x;
		expression.func_args = args;
		return expression.expr();
	}

}

//using these std::maps with lambdas (in calc_nodes()) is slower, than comparing to strings
//std::map<std::string, std::function<std::function<float()>(std::function<float()>, std::function<float()>)>> op_set{
//	{"+",
//		[](const std::function<float()>& left, const std::function<float()>& right)->std::function<float()> {
//			return [=]() {
//				return left()+right();
//			};
//		}
//	}, {"-",
//		[](const std::function<float()>& left, const std::function<float()>& right)->std::function<float()> {
//			return [=]() {
//				return left()-right();
//			};
//		}
//	}, {"*",
//		[](const std::function<float()>& left, const std::function<float()>& right)->std::function<float()> {
//			return [=]() {
//				return left()*right();
//			};
//		}
//	}, {"/",
//		[](const std::function<float()>& left, const std::function<float()>& right)->std::function<float()> {
//			return [=]() {
//				return left()/right();
//			};
//		}
//	}, {"^",
//		[](const std::function<float()>& left, const std::function<float()>& right)->std::function<float()> {
//			return [=]() {
//				return pow(left(), right());
//			};
//		}
//	} };
//std::map<std::string, std::function<std::function<float()>(std::function<float()>)>> cop_set{
//	{"log",
//		[](const std::function<float()>& func)->std::function<float()> {
//			return [=]() {
//				return std::log(func());
//			};
//		}
//	}, {"sin",
//		[](const std::function<float()>& func)->std::function<float()> {
//			return [=]() {
//				return sin(func());
//			};
//		}
//	}, {"cos",
//		[](const std::function<float()>& func)->std::function<float()> {
//			return [=]() {
//				return cos(func());
//			};
//		}
//	}, {"tan",
//		[](const std::function<float()>& func)->std::function<float()> {
//			return [=]() {
//				return tan(func());
//			};
//		}
//	}, {"sqrt",
//		[](const std::function<float()>& func)->std::function<float()> {
//			return [=]() {
//				return sqrt(func());
//			};
//		}
//	} };
