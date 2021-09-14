
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <queue>
#include <chrono>

#include "functions.hpp"


const std::string algebraic_expression::dependent_start = "__x";
const std::string algebraic_expression::dependent_start_function = "__f";
const char algebraic_expression::function_marker = '!';
const std::string algebraic_expression::function_marker_string(1, algebraic_expression::function_marker);

      expression::eq_node::eq_node(const std::string &tok_name) : token_name(tok_name) { }

      expression::error_handling::error_handling(error_code er) : error(er) {}

      expression::expression() : root(nullptr), parsing_error(error_handling::empty) { }

      expression::expression(const std::string& s) : root(nullptr), parsing_error(error_handling::empty) { parse(s);  }

      expression::expression(long long i) : root(nullptr), parsing_error(error_handling::empty) { parse(std::to_string(i)); }

      expression::expression(eq_node* root_to_copy) : parsing_error(error_handling::empty) { root = copy_subtree(root_to_copy);  }

      expression::expression(const expression& e) : parsing_error(e.parsing_error) { root = copy_subtree(e.root);  }

      expression::expression(expression&& e) : parsing_error(e.parsing_error), root(e.root) { e.root = nullptr; }

      expression& expression::operator=(expression&& e)  {
          delete_node(root);
          parsing_error = e.parsing_error;
          root = e.root;
          e.root = nullptr;
          return *this;
      }

      expression& expression::operator=(const expression& e) {
          if (this != &e) {
              parsing_error = e.parsing_error;
              delete_node(root);
              root = copy_subtree(e.root);
          }
          return *this;
      }

      expression::~expression() {
          delete_node(root);
      }

      bool expression::operator==(const expression& b) const {
          expression temp(*this);
          temp = (temp - b).simplify_rational();
          bool ret = (temp.root->token_name == "0");
          return ret;
      }


      expression& expression::parse(std::string s, const std::vector<std::string>& variables) {
          // parses exression; allowed chars: (),,,0-9,a-z,_,+,-,*,/,^,\ and space; -,\ can act as unary or binary operators; the rest only binary
          // space between tokens interpreted as product, ()() interpreted as product, token() interpreted as function, unless token is a number
          // variables is used to turn expressions x(4) from unary functions x into multiplication x*4

          std::list<std::pair<char,int>> s_temp;
          for(int i = 0; i < s.size(); ++i) {
              s_temp.push_back(std::make_pair(s[i], i));
          }
          s_temp.push_back(std::make_pair(' ', s.size()));

          // insert space between numeric '0'-'9' and nonnumeric -- this has effect that 2x^2 is interpreted as 2 x^2
          auto it = s_temp.begin();
          while (next(it)!=s_temp.end()) {
              if ((it->first >= '0') && (it->first <= '9') && ((next(it)->first < '0') || (next(it)->first > '9')) && (next(it)->first != '.')) {
                  it = s_temp.insert(next(it), std::make_pair(' ', it->second));
              }
              ++it;
          }

          // validate

          enum status { needtoken, midtoken, endtoken };
          enum action_after { only_binary, binary_comma, terminate };

          std::stack<std::pair<status, action_after>> expression_stack;
          expression_stack.push(std::make_pair(needtoken, only_binary));
          parsing_error.error = error_handling::ok;
          it = s_temp.begin();
          while (it!=s_temp.end()) {
              char c = it->first;
              parsing_error.position_of_error = it->second;
              if (!is_allowed_char(c) && (c!=' ')) {
                  parsing_error.error = error_handling::disallowed_char;
                  return *this;
              }
              if (expression_stack.top().first==midtoken) {
                  if (is_token_char(c)) {    ++it; continue;    }
                  expression_stack.top().first = endtoken;
                  if (c=='(') {
                      expression_stack.push(std::make_pair(needtoken, binary_comma)); ++it; continue;
                  }
              }
              if (expression_stack.top().first==needtoken) {
                  if ((c==')') || (c==',') || ((is_binary_operator(c)) && (!is_unary_operator(c)))) {
                      parsing_error.error = error_handling::token_expected; 
                      return *this;
                  }
                  if (is_token_char(c)) {
                      expression_stack.top().first = midtoken;
                  }
                  if (is_unary_operator(c)) {
                      expression_stack.push(std::make_pair(needtoken, terminate));
                  }
                  if (c=='(') {
                      expression_stack.push(std::make_pair(needtoken, only_binary));
                  }
              }
              if (expression_stack.top().first==endtoken) {
                  if (expression_stack.top().second==terminate) {
                      expression_stack.pop(); 
                      expression_stack.top().first = endtoken;
                      continue;
                  }
                  if ((c==',') && (expression_stack.top().second!=binary_comma)) {
                      parsing_error.error = error_handling::stray_comma; 
                      return *this;
                  }
                  if (c==')') {
                      expression_stack.pop(); if (expression_stack.empty()) { parsing_error.error = error_handling::redundant_closing_bracket; 
                                                                                   return *this; }
                      expression_stack.top().first = endtoken;
                  }
                  if ((c==',') || (is_binary_operator(c))) {
                      expression_stack.top().first = needtoken;
                  }
                  if ((c=='(') || (is_token_char(c))) {
                      expression_stack.top().first = needtoken;
                      continue;
                  }
              }
              ++it;
          }
          if ((expression_stack.size()!=1) || (expression_stack.top().first != endtoken)) { 
              parsing_error.error = error_handling::ends_too_soon; 
              return *this; 
          }

          // turn unary operators -,\ into functions; also turn spaces between tokens into multiplication

          std::stack<std::pair<status, action_after>> last_unary;
          last_unary.push(std::make_pair(needtoken, only_binary));
          it = s_temp.begin();
          while (it!=s_temp.end()) {
              char c = it->first;
              if (last_unary.top().first==midtoken) {
                  if (is_token_char(c)) {  ++it;  continue;    }
                  last_unary.top().first = endtoken;
                  if (c=='(') {
                     last_unary.push(std::make_pair(needtoken, binary_comma)); ++it; continue;
                  }
              }
              if (last_unary.top().first==needtoken) {
                  if (is_token_char(c)) {
                      last_unary.top().first = midtoken;
                  }
                  if (is_unary_operator(c)) {
                      last_unary.push(std::make_pair(needtoken, terminate));
                      std::string to_insert = function_name(it->first, unary_operators);
                      it = s_temp.erase(it);
                      for(int i = to_insert.size()-1; i >= 0; --i) {
                        it = s_temp.insert(it, std::make_pair(to_insert[i],0));
                      }
                      it = next(it, to_insert.size()-1);
                  }
                  if (c=='(') {
                      last_unary.push(std::make_pair(needtoken, only_binary));
                  }
              }
              if (last_unary.top().first==endtoken) {
                  if (last_unary.top().second==terminate) {
                      last_unary.pop(); last_unary.top().first = endtoken;
                      s_temp.insert(it, std::make_pair(')',0));
                      continue;
                  }
                  if (c==')') {
                      last_unary.pop();
                      last_unary.top().first = endtoken;
                  }
                  if ((c==',') || (is_binary_operator(c))) {
                      last_unary.top().first = needtoken;
                  }
                  if ((c=='(') || (is_token_char(c))) {
                      it = s_temp.insert(it, std::make_pair('*',0));
                      last_unary.top().first = needtoken;
                  }
              }
              ++it;
          }

          // erase spaces

          it = s_temp.begin();
          while (it!=s_temp.end()) {
              if (it->first == ' ') {   it = s_temp.erase(it);  } else { ++it; }
          }
          s_temp.insert(s_temp.begin(), std::make_pair('(', 0)); s_temp.push_back(std::make_pair(')', 0));

          // turn binary operators into functions according to precedence

          for(int precedence_level = binary_operators.size()-1; precedence_level >= 0; --precedence_level) {
              std::stack<std::tuple<std::list<std::pair<char,int>>::iterator,
                         std::list<std::pair<char,int>>::iterator,int>> last_token;   // tuple<beginning of last token, binary operator, depth>
              int depth = 0;
              auto binary_row = binary_operators[precedence_level];
              auto it = s_temp.begin();
              while (it != s_temp.end()) {
                  if (is_token_char(it->first)) {
                      if ((last_token.empty()) || (std::get<2>(last_token.top()) != depth)) {
                        last_token.push(make_tuple(it, s_temp.end(), depth));
                      }
                  }
                  if (it->first=='(') {
                      if ((last_token.empty()) || (std::get<2>(last_token.top()) != depth)) {
                        last_token.push(make_tuple(it, s_temp.end(), depth));
                      }
                      ++depth; 
                  }
                  if (is_binary_operator(it->first)) {
                      if (std::get<1>(last_token.top()) != s_temp.end()) {
                          auto it1 = modify_expression(s_temp, it, last_token, binary_row);
                          std::get<0>(last_token.top()) = it1;
                          std::get<1>(last_token.top()) = s_temp.end();
                      }
                      if (is_binary_operator_helper(it->first, binary_row)) {
                          std::get<1>(last_token.top()) = it;
                      } else {
                          last_token.pop();
                      }
                  }
                  if ((it->first==')') || (it->first==',')) {
                      if (std::get<1>(last_token.top()) != s_temp.end()) {
                          modify_expression(s_temp, it, last_token, binary_row);
                      }
                      last_token.pop();
                      if (it->first==')') {
                        --depth;
                      }
                  }
                  ++it;
              }
          }

          // transform expression into expression graph

          it = s_temp.begin();
          std::string last_tok;
          std::stack<std::pair<bool,eq_node*>> processed_so_far;
          while (it!=s_temp.end()) {
              if (is_token_char(it->first)) {
                  last_tok += it->first; ++it; continue;
              }
              if ((!last_tok.empty()) && (it->first != '(')) {
                  it = s_temp.insert(it, std::make_pair(')',0)); it = s_temp.insert(it, std::make_pair('(',0)); continue;
              }
              if ((it->first=='(') && (last_tok.empty())) {
                  last_tok = id_func.name;
              }

              if (it->first=='(') {
                  processed_so_far.push(std::make_pair(false, new eq_node(last_tok)));
              }
              if (it->first==')') {
                  std::stack<eq_node*> arguments;
                  while (processed_so_far.top().first) {
                      arguments.push(processed_so_far.top().second);
                      processed_so_far.pop();
                  }
                  processed_so_far.top().first = true;
                  while (!arguments.empty()) {
                      processed_so_far.top().second->children.push_back(arguments.top());
                      arguments.pop();
                  }
              }

              last_tok = "";
              ++it;
          }
          root = processed_so_far.top().second;

          delete_identity(root);

          // turn "functions" 2(x) into multiplications 2 (x)
          auto arg_tokens = get_tokens();
          if (arg_tokens.size()>1) {
            for(auto tok : arg_tokens[1]) {
              if ((!big_num::is_num(tok)) && (find(variables.begin(), variables.end(), tok) == variables.end()))  { continue; }
              post_order_crawler(root, [tok](eq_node* root){
                if ((root->token_name==tok) && (root->children.size()==1)) {
                    eq_node* new_node = new eq_node(tok);
                    root->token_name = multiply_func.name;
                    root->children.push_back(new_node);
                }
              });
            }
          }

          return *this;
      }

      expression& expression::substitute(const std::string& var_name, const expression& new_expr) {
          // var_name must be a variable, ie 0-argument function (no children in tree) !!!! undefined behavior if not!!
          substitute_helper(root, var_name, new_expr);
          if (root->token_name==var_name) {
             expression temp_expr(new_expr);
             delete root;
             root = temp_expr.root;
             temp_expr.root = nullptr;
          }
          return *this;
      }

      std::vector<std::vector<std::string>> expression::get_tokens() const {
          // returns tokens in array; array[i] contains tokens with i arguments
          std::vector<std::set<std::string>> tokens_as_set;
          post_order_crawler(root, [&tokens_as_set](eq_node* root){
                while (tokens_as_set.size() < root->children.size()+1) {
                    tokens_as_set.push_back(std::set<std::string>());
                }
                tokens_as_set[root->children.size()].insert(root->token_name);
          });
          std::vector<std::vector<std::string>> ret;
          for(auto toks : tokens_as_set) {
              ret.push_back(std::vector<std::string>(toks.begin(), toks.end()));
          }
          return ret;
      }

      expression expression::operator+(const expression& b) const {
          expression a1(*this), a2(b);
          expression ret;
          ret.root = new eq_node(plus_func.name);
          ret.root->children.push_back(a1.root); a1.root = nullptr;
          ret.root->children.push_back(a2.root); a2.root = nullptr;
          return ret;
      }

      expression expression::operator-(const expression& b) const {
          expression a1(*this), a2(b);
          expression ret;
          ret.root = new eq_node(minus_func.name);
          ret.root->children.push_back(a1.root); a1.root = nullptr;
          ret.root->children.push_back(a2.root); a2.root = nullptr;
          return ret;
      }

      expression expression::operator*(const expression& b) const {
          expression a1(*this), a2(b), ret;
          ret.root = new eq_node(multiply_func.name);
          ret.root->children.push_back(a1.root); a1.root = nullptr;
          ret.root->children.push_back(a2.root); a2.root = nullptr;
          return ret;
      }

      expression expression::operator/(const expression& b) const {
          expression a1(*this), a2(b), ret;
          ret.root = new eq_node(divide_func.name);
          ret.root->children.push_back(a1.root); a1.root = nullptr;
          ret.root->children.push_back(a2.root); a2.root = nullptr;
          return ret;
      }

      expression expression::operator-() const {
          expression a(*this), ret;
          ret.root = new eq_node(minus_unary_func.name);
          ret.root->children.push_back(a.root); a.root = nullptr;
          return ret;
      }

      expression expression::operator^(const expression& b) const {
          expression a1(*this), a2(b), ret;
          ret.root = new eq_node(power_func.name);
          ret.root->children.push_back(a1.root); a1.root = nullptr;
          ret.root->children.push_back(a2.root); a2.root = nullptr;
          return ret;
      }

      // simplifies expression, divides polynomials if cancel_var != ""
      expression& expression::simplify_rational(const std::string& cancel_var) {

          check(root!=nullptr) << "simplify called on nullptr expression\n";

          algebraic_expression alg_exp(*this, cancel_var);
          if ((alg_exp.is_ok) && ( (alg_exp.denom_coeff.size()<=1) || (alg_exp.num_coeff.size()<=1) || (cancel_var.empty()) )) {
              expression ret;
              ret = alg_exp.to_expression();
              root = ret.root;
              ret.root = nullptr;
          } else if (alg_exp.is_ok) {
              std::vector<expression> num_large, denom_large;
              for(const auto& x : alg_exp.num_coeff) {
                  num_large.push_back(x.to_expression(alg_exp.dependent_vars, alg_exp.func_arg));
              }
              for(const auto& x : alg_exp.denom_coeff) {
                  denom_large.push_back(x.to_expression(alg_exp.dependent_vars, alg_exp.func_arg));
              }
              expression ret;
              auto gcd = polynomial::gcd(num_large, denom_large);
              if (gcd.size()>1) {
                auto num = polynomial::divmod(num_large, gcd).first;
                auto denom = polynomial::divmod(denom_large, gcd).first;
                for(auto& c : num) { c.simplify_rational(); }
                for(auto& c : denom) { c.simplify_rational(); }
                expression num_expr = polynomial::to_expression(num, cancel_var);
                expression denom_expr = polynomial::to_expression(denom, cancel_var);
                ret = (num_expr/denom_expr).simplify_rational("x");
              } else {
                ret = alg_exp.to_expression();
              }
              root = ret.root;
              ret.root = nullptr;
          }

          return *this;
      }

      void expression::print() const {
          print_helper(root);
          std::cout << std::endl;
          return;
      }

      void expression::print_raw() const {
          print_helper_raw(root);
          std::cout << std::endl;
          return;
      }

      void expression::post_order_crawler(eq_node* root, const std::function<void(eq_node*)>& process) {
            if (root!=nullptr) {
                for(auto c_n : root->children) {
                    post_order_crawler(c_n, process);
                }
                process(root);
            }            
       }


        void expression::substitute_helper(eq_node* root, const std::string& var_name, const expression& new_expr) {
            post_order_crawler(root, [&new_expr,&var_name](eq_node* root){
                if (root->token_name==var_name) {
                    expression temp_expr(new_expr);
                    eq_node* temp_node = temp_expr.root;
                    *root = *temp_node;
                    temp_expr.root->children.clear();
                }
            });
        }

        void expression::print_helper(eq_node* root) {
          if (root!=nullptr) {
            char delimiter = ',';
            if (root->token_name==multiply_func.name) {
                delimiter = '*';
            } else if (root->token_name==divide_func.name) {
                delimiter = '/';
            } else if (root->token_name==plus_func.name) {
                delimiter = '+';
            } else if (root->token_name==minus_func.name) {
                delimiter = '-';
            } else {   std::cout << root->token_name;   }
            if ((!root->children.empty()) && ((root->token_name!=divide_func.name) && (root->token_name!=multiply_func.name))) {
              std::cout << "(";
            }
            for(int i = 0; i < root->children.size(); ++i) {
                print_helper(root->children[i]);
                if (i < root->children.size()-1) {
                  std::cout << delimiter;
                }
            }
            if ((!root->children.empty()) && ((root->token_name!=divide_func.name) && (root->token_name!=multiply_func.name))) {
              std::cout << ")";
            }
          }
        }

        void expression::print_helper_raw(eq_node* root) {
          if (root!=nullptr) {
            std::cout << root->token_name << "(";
            for(int i = 0; i < root->children.size(); ++i)  {
                print_helper_raw( root->children[i]);
                if (i+1 < root->children.size())  std::cout << ",";
            }
            std::cout << ")";
          }
        }


        void expression::delete_identity(eq_node* root) {
            post_order_crawler(root, [](eq_node* root){
                if (root->token_name == id_func.name) {
                    eq_node* temp_node = root->children.front();
                    *root = *temp_node;
                    delete temp_node;
                } 
            });
        }

        void expression::delete_node(eq_node* root) {
            post_order_crawler(root, [](eq_node* root){
                delete root;
            });
        }

        expression::eq_node* expression::copy_subtree(eq_node* root) {
            if (root!=nullptr) { 
              eq_node* ret = new eq_node(root->token_name);
              for(auto c_n : root->children) {
                ret->children.push_back(copy_subtree(c_n));
              }
              return ret;
            }
            return nullptr;
        }

        bool expression::is_allowed_char(char c) {
            return (is_token_char(c) || is_binary_operator(c) || (c=='(') || (c==')') || (c==','));
        }

        bool expression::is_token_char(char c) {
            return token_alphabet.count(c)>0;
        }

        bool expression::is_binary_operator(char c) {
            bool found = false;
            for(auto bin_row : binary_operators) {
                found |= is_binary_operator_helper(c, bin_row);
            }
            return found;
        }

        bool expression::is_unary_operator(char c) {
            bool found = false;
            for(auto u : unary_operators) {
                found |= (c==u.first);
            }
            return found;
        }

        bool expression::is_binary_operator_helper(char c, std::vector<std::pair<char, std::string>>& bin_row) {
            bool found = false;
            for(auto b : bin_row) {
                found |= (b.first==c);
            }
            return found;
        }

        std::string expression::function_name(char c, const std::vector<std::pair<char, std::string>>& bin_row) {
            for(auto b : bin_row) {
                if (b.first==c) {
                    return b.second+"(";
                }
            }
            return "";
        }

        std::list<std::pair<char,int>>::iterator expression::modify_expression(std::list<std::pair<char,int>>& s_temp, std::list<std::pair<char,int>>::iterator it, 
                                               std::stack<std::tuple<std::list<std::pair<char,int>>::iterator, std::list<std::pair<char,int>>::iterator,int>>& last_token, 
                                               const std::vector<std::pair<char,std::string>>& binary_row) {
            s_temp.insert(it, std::make_pair(')',0));
            std::string to_insert = function_name(std::get<1>(last_token.top())->first, binary_row);
            std::get<1>(last_token.top())->first = ',';
            auto it1 = std::get<0>(last_token.top());
            for(int i = to_insert.size()-1; i >= 0; --i) {
               it1 = s_temp.insert(it1, std::make_pair(to_insert[i],0));
            }
            return it1;
        }


              thread_local std::chrono::time_point<std::chrono::steady_clock> time_start;
              thread_local int timing = 0;
              thread_local int time_limit = 3;

              algebraic_expression::algebraic_expression(expression ex, const std::string& var, int tl)  {

            // a+b -> a+(-b)
                expression::post_order_crawler(ex.root, [](expression::eq_node* root) {
                  if (root->token_name == minus_func.name) {
                    root->token_name = plus_func.name;
                    expression::eq_node* unary_minus = new expression::eq_node(minus_unary_func.name);
                    unary_minus->children.push_back(root->children[1]);
                    root->children[1] = unary_minus;
                  }
                });

            // -b -> (-1)*b
                expression::post_order_crawler(ex.root, [](expression::eq_node* root) {
                  if (root->token_name == minus_unary_func.name) {
                    root->token_name = multiply_func.name;
                    root->children.push_back(new expression::eq_node("-1"));
                  }
                });

            // sqrt_unary -> sqrt( ,2)
                expression::post_order_crawler(ex.root, [](expression::eq_node* root) {
                  if (root->token_name == sqrt_unary_func.name) {
                    root->token_name = sqrt_func.name;
                    root->children.push_back(new expression::eq_node("2"));
                    std::swap(root->children[0], root->children[1]);
                  }
                });

            // sqrt(,x) -> pow( ,1/x)
                expression::post_order_crawler(ex.root, [](expression::eq_node* root) {
                  if (root->token_name == sqrt_func.name) {
                    root->token_name = power_func.name;
                    std::swap(root->children[0], root->children[1]);
                    expression::eq_node* div = new expression::eq_node(divide_func.name);
                    div->children.push_back(new expression::eq_node("1"));
                    div->children.push_back(root->children[1]);
                    root->children[1] = div;
                  }
                });

                time_start = std::chrono::steady_clock::now();
                timing = 1234567;
                time_limit = tl;
                is_ok = true;
                const int MAX_DEGREE_POL = 20;
                try {
                   value = expand(common_denominator(ex.root), 1);
                   // extract polynomial coefficients for numerator and denominator
                   if (!var.empty()) {
                       std::vector<std::vector<term>*> numdenom{ &value.numerator, &value.denominator };
                       std::vector<std::vector<fraction>*> numdenom_coeff{ &num_coeff, &denom_coeff };
                       // the following is because the fraction need not be cancelled!!
                       auto temp_min = value.extract_factor(var);
                       big_num_rational base = std::min(temp_min.first, temp_min.second);
                       for(int i = 0; i < 2; ++i) {
                           int power;
                           for(auto t : *numdenom[i]) {
                               if (t.vars.index.count(var)>0) {
                                   auto temp_power = t.vars.index[var].extract_rational_number();
                                   if (temp_power.first && (big_num_rational::is_int(temp_power.second-base)) &&
                                       (temp_power.second-base<=MAX_DEGREE_POL)) {
                                           power = std::stoi((temp_power.second-base).numerator.to_string());
                                       } else {
                                           power = -1;
                                       }
                                   t.vars.index.erase(var);
                               } else {
                                   power = std::stoi((-base).numerator.to_string());
                                   if (base.denominator!=1) {
                                       power = -1;
                                   }
                               }
                               auto c = fraction(std::vector<term>{ t }, std::vector<term>{ term(1) });
                               if ((c.to_expression(dependent_vars, func_arg).dependent_on(var)) || (power<0)) {
                                   power = -1;
                               } else {
                                   while (numdenom_coeff[i]->size() < power+1) {
                                       numdenom_coeff[i]->push_back(fraction(0LL));
                                   }
                                   numdenom_coeff[i]->operator[](power) = numdenom_coeff[i]->operator[](power) + c;
                               }
                               if (power==-1) {
                                   break;
                               }
                           }
                           if (power==-1) {
                               numdenom_coeff[i]->clear();
                           }
                       }
                   }
                } catch (const std::string& e) {
                   value = fraction(0);
                   is_ok = false;
                }
                timing = 0;
              }

              algebraic_expression::multiindex::multiindex(const std::map<std::string, fraction>& ind_) : index(ind_) {}

              algebraic_expression::multiindex algebraic_expression::multiindex::operator+(const multiindex& b) const {
                  multiindex ret(index);
                  for(const auto& var : b.index) {
                      ret.index[var.first] = ret.index[var.first] + var.second;
                  }
                  ret.simplify();
                  return ret;
              }


              algebraic_expression::multiindex& algebraic_expression::multiindex::simplify() {
                  auto it = index.begin();
                  while (it!=index.end()) {
                      if (it->second==fraction(0)) {
                          it = index.erase(it);
                      } else {
                          ++it;
                      }
                  }
                  return *this;
              }


              algebraic_expression::term::term(const multiindex& mi, const big_num_rational& bn) : coeff(bn), vars(mi) {}

              algebraic_expression::term::term(const big_num_rational& bn) : coeff(bn) {}

              // can only be called with token_name a variable (==zero arg function)
              algebraic_expression::term::term(std::string token_name, const fraction& power) : coeff(1) {
                  vars.index[token_name] = power; simplify();   
              }

              algebraic_expression::term::term(const big_pos_int& number, const fraction& power) : coeff(1) {
                  vars.index[number.to_string()] = power; simplify();
              }

              algebraic_expression::term algebraic_expression::term::operator*(const term& b) const {
                  return term(vars + b.vars, coeff * b.coeff).simplify();
              }

              algebraic_expression::term algebraic_expression::term::operator/(term b) const {
                  check(b.coeff!=0) << "term divide called with denominator 0";
                  b.coeff = big_num_rational(1)/b.coeff;
                  for(auto& f : b.vars.index) { f.second = f.second*fraction(-1);  }
                  return *this * b;
              }

               // TODO:sqrt(x^2) == abs(x) -- insert logic here!!
             algebraic_expression::term algebraic_expression::term::operator^(const fraction& pow) const {
                  term temp(*this);
                  big_num_rational number(temp.coeff);
                  temp.coeff = 1;
                  for(auto& t : temp.vars.index) { t.second = t.second * pow; }
                  if (number<0) { 
                      number = -number; 
                      auto pow_rat = pow.extract_rational_number();
                      if (pow_rat.first) {
                        if (pow_rat.second.denominator%2==1) {
                            if (pow_rat.second.numerator%2!=0) { temp.coeff = temp.coeff * (-1); }
                        } else {
                            temp = temp * term(imaginary_unit.name, fraction(2)*pow);
                        }
                      } else {
                        temp = temp * term(imaginary_unit.name, fraction(2)*pow);
                      }
                  }
                  temp = temp * term(abs(number.numerator), pow);
                  temp = temp / term(number.denominator, pow);
                  return temp;
              }

              bool algebraic_expression::term::operator==(const term& b) const {
                  auto q = (*this)/b;
                  return (q.vars.index.empty()) && (q.coeff==1);
              }

              // returns true if quotient is a rational number; used in simplify_sum(vector<term>) and extract_rational
              bool algebraic_expression::term::same_vars(const term& a, const term& b) {
                  return (a/b).vars.index.empty();
              }

              // returns true if one ln in products
              bool algebraic_expression::term::has_one_ln() const {
                  auto it = vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string);
                  bool ret = (it != vars.index.end()) && 
                             (it->first.substr(0,2+ln_func.name.size()) == function_marker_string + ln_func.name + function_marker_string) &&
                             ((next(it) == vars.index.end()) || 
                              (next(it)->first.substr(0,2+ln_func.name.size()) != function_marker_string + ln_func.name + function_marker_string)) &&
                             (it->second==fraction(1));
                  return ret;
              }

              // returns true if no logarithms present
              bool algebraic_expression::term::has_no_ln() const {
                  auto it = vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string);
                  return ((it==vars.index.end()) || (it->first.substr(0,2+ln_func.name.size())!=function_marker_string + ln_func.name + function_marker_string));
              }

              // splits numerical factors in ind into pairwise prime factors, modifies exponents accordingly
              void algebraic_expression::term::pairwise_prime(std::map<std::string, fraction>& ind) {
                  std::vector<big_pos_int> facts;
                  for(const auto& x : ind) { if (big_num::is_int(x.first)) { facts.push_back(x.first); }  }
                  facts = factors(facts);
                  std::map<std::string, fraction> new_varsindex;
                  for(const auto& x : ind) {
                      if (big_num::is_int(x.first)) {
                          for(const auto& p : facts) {
                              big_pos_int base = x.first;
                              int k = 0;
                              while (base % p == 0) { ++k; base = base/p;  }
                              if (k>0) {
                                  new_varsindex[p.to_string()] = new_varsindex[p.to_string()] + fraction(k) * x.second;
                              }
                          }
                      } else {
                          new_varsindex[x.first] = x.second;
                      }
                  }
                  ind = move(new_varsindex);
              }

              algebraic_expression::term& algebraic_expression::term::simplify() {
                  auto curr = std::chrono::steady_clock::now();
                  std::chrono::duration<double> diff = curr-time_start;
                  if ((diff.count()>time_limit) && (timing==1234567)) {
                      throw std::string("Timeout!");
                  }
                  auto it = vars.index.begin();
                  while (it!=vars.index.end()) {
                      if ((it->first!=euler_number.name) && (it->second.denominator.size()==1)) {
                          std::vector<term> num_new;
                          for(const auto& t : it->second.numerator) {
                              term t_ = t/it->second.denominator.front();
                              if (t_.has_no_ln()) {
                                  num_new.push_back(t);
                              } else {
                                  if (it->first != "1") {
                                    t_ = t_ * term( function_marker_string + ln_func.name + function_marker_string + it->first, fraction(1) );
                                    vars.index[euler_number.name] = vars.index[euler_number.name] + 
                                                                    fraction(std::vector<term>{ t_ }, std::vector<term>{ term(1) });
                                  }
                              }
                          }
                          it->second.numerator = num_new;
                      }
                      ++it;
                  }
                  if ((vars.index.count(euler_number.name)>0) && (vars.index[euler_number.name].denominator.size()==1)) {
                      std::vector<term> num_new;
                      for(const auto& t : vars.index[euler_number.name].numerator) {
                          term t_ = t/vars.index[euler_number.name].denominator.front();
                          if (t_.has_one_ln()) {
                              std::string var_name = t_.vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string)->first;
                              var_name = var_name.substr(2 + ln_func.name.size());
                              t_.vars.index.erase(t_.vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string));
                              vars.index[var_name] = vars.index[var_name] + 
                                                     fraction(std::vector<term>{ t_ }, std::vector<term>{ term(1) });
                          } else {
                              num_new.push_back(t);
                          }
                      }
                      vars.index[euler_number.name].numerator = num_new;
                  }

                  pairwise_prime(vars.index);

                  it = vars.index.begin();
                  while (it!=vars.index.end()) {
                      auto rat_num = it->second.extract_rational_number();
                      if ((rat_num.first) && (big_int::is_num(it->first))) {

                      // check you dont get large numbers here
                          if (abs(rat_num.second) * big_num_rational(it->first.size())>150) {
                              ++it; continue;
                          }

                          big_pos_int pow_int = abs(rat_num.second.numerator)/rat_num.second.denominator;
                          big_pos_int res = big_pos_int(it->first)^pow_int;
                          if (rat_num.second > 0) {
                              coeff = coeff * big_num(res);
                              it->second = it->second + fraction(-1) * fraction(big_num(pow_int));
                          } else {
                              coeff = coeff / big_num(res);
                              it->second = it->second + fraction(big_num(pow_int));
                          }
                          if (it->second==fraction(0)) {
                            it = vars.index.erase(it);
                          } else {
                            ++it;
                          }
                      } else {
                          ++it;
                      }
                  }

                  return *this;
              }

              std::vector<big_pos_int> algebraic_expression::term::factors(const std::vector<big_pos_int>& a) {
                std::list<big_pos_int> ret;
                std::queue<big_pos_int> todo;
                for(auto x : a) { todo.push(x); }
                while (!todo.empty())  {
                  auto curr = todo.front(); todo.pop();
                  if (curr==1) { continue; }
                  bool enough = false;
                  for(auto it=ret.begin(); it != ret.end(); ++it) {
                    big_pos_int x = big_pos_int::gcd(*it, curr);
                    if (x!=1) {
                      *it = *it/x;
                      if (*it==1) { it = ret.erase(it); }
                      todo.push(x);
                      todo.push(curr/x);
                      enough = true;
                      break;
                    }
                  }
                  if (!enough) {
                    ret.push_back(curr);
                  }
                }
                for(auto& x : ret) {
                    for(int i = x.to_string().size()*3; i >= 2; --i) {
                        auto temp = x.take_root(i);
                        if (temp.first) {
                            x = temp.second;
                            break;
                        }
                    }
                } 
                return std::vector<big_pos_int>(ret.begin(), ret.end());
              }


              expression algebraic_expression::term::to_expression(const map_bf<fraction,int>& dep_var, 
                                                                   const std::map<std::string, map_bf<fraction,int>>& dep_var_func) const {
                  term term_copy(*this); term_copy.simplify();
                  expression ret;
                  ret.root = new expression::eq_node(multiply_func.name);
                  if ((term_copy.coeff!=1) || (term_copy.vars.index.empty())) {
                    if (term_copy.coeff.denominator!=1) {
                      expression temp = expression(term_copy.coeff.numerator.to_string())/expression(term_copy.coeff.denominator.to_string());
                      ret.root->children.push_back(temp.root);
                      temp.root = nullptr;
                    } else {
                      ret.root->children.push_back(new expression::eq_node(term_copy.coeff.numerator.to_string()));
                    }
                  }
                  for(const auto& var : term_copy.vars.index) {
                      expression base;
                      std::string base_name(var.first);
                      std::stack<std::string> functions;
                      // TODO: multivariable functions, insert support here
                      while (base_name[0]==function_marker) {
                          int next_exclamation = find(base_name.begin()+1, base_name.end(), function_marker) - base_name.begin();
                          functions.push(base_name.substr(1, next_exclamation-1));
                          base_name = base_name.substr(next_exclamation+1);
                      }
                      if (base_name.substr(0, dependent_start.size())==dependent_start) {
                          base = dep_var.at(stoi(base_name.substr(dependent_start.size()))).to_expression(dep_var, dep_var_func);
                      } else if (base_name.substr(0, dependent_start_function.size())==dependent_start_function) {
                          std::string temp_name = base_name.substr(dependent_start_function.size());
                          int i = std::find(temp_name.begin(), temp_name.end(), function_marker) - temp_name.begin();
                          std::string func_name = temp_name.substr(0, i);
                          i = stoi(temp_name.substr(i+1));
                          base = dep_var_func.at(func_name).at(i).to_expression(dep_var, dep_var_func);
                          expression::eq_node* f_name = new expression::eq_node(func_name);
                          f_name->children.push_back(base.root);
                          base.root = f_name;
                      } else {
                          base = expression(base_name);
                      }
                      while (!functions.empty()) {
                          auto new_f = new expression::eq_node(functions.top());
                          new_f->children.push_back(base.root);
                          base.root = new_f;
                          functions.pop();
                      }
                      auto q_rat = var.second.extract_rational_number();
                      if (var.second==fraction(big_num_rational(1))) {
                        ret.root->children.push_back(base.root);
                      } else if ((q_rat.first) && (q_rat.second.denominator!=1)) {
                        if (q_rat.second.denominator==2) {
                          ret.root->children.push_back(new expression::eq_node(sqrt_unary_func_name));
                        } else {
                          ret.root->children.push_back(new expression::eq_node(sqrt_func_name));
                          ret.root->children.back()->children.push_back(new expression::eq_node(q_rat.second.denominator.to_string()));
                        }
                        if (q_rat.second.numerator==1) {
                          ret.root->children.back()->children.push_back(base.root);
                        } else {
                          ret.root->children.back()->children.push_back(new expression::eq_node(power_func_name));
                          ret.root->children.back()->children.back()->children.push_back(base.root);
                          ret.root->children.back()->children.back()->children.push_back(new expression::eq_node(q_rat.second.numerator.to_string()));
                        }
                      } else {
                        ret.root->children.push_back(new expression::eq_node(power_func_name));
                        ret.root->children.back()->children.push_back(base.root);
                        expression pow = var.second.to_expression(dep_var, dep_var_func);
                        ret.root->children.back()->children.push_back(pow.root);
                        pow.root = nullptr;
                      }
                      base.root = nullptr;
                  }
                  check(!ret.root->children.empty()) << "term to_expression produces empty expression\n";
                  if (ret.root->children.size()==1) {
                      ret = expression(ret.root->children.front());
                  }
                  return ret;
              }


        // TODO: to speed up things: try implementing big_num as ll for small numbers, implement vector as array for small sizes, store variables
        // as strings separately in a dicitonary and have them indexed by numbers, so you can have map in term replaced with vector,
        // have fraction comparator compare value at, say x=1
        // somehow try dividing numerator with denominator if possible, at least for quotient equal to rational number

              algebraic_expression::fraction::fraction() : denominator{ term(1) } {}

              algebraic_expression::fraction::fraction(const std::vector<term>& a, const std::vector<term>& b) : numerator(a), denominator(b) {}

              algebraic_expression::fraction::fraction(const big_num_rational& a) : denominator{ term(1) }  {
                if (a!=0) {
                  numerator = std::vector<term>{ term(a) };
                }
              }

              algebraic_expression::fraction algebraic_expression::fraction::operator*(const fraction& b) const {
                  return fraction(multiply(numerator, b.numerator), multiply(denominator, b.denominator)).simplify();
              }

              algebraic_expression::fraction algebraic_expression::fraction::operator/(const fraction& b) const {
                  return fraction(multiply(numerator, b.denominator), multiply(denominator, b.numerator)).simplify();
              }

              algebraic_expression::fraction algebraic_expression::fraction::operator+(const fraction& b) const {
                  std::vector<term> temp1(multiply(numerator, b.denominator)), temp2(multiply(b.numerator, denominator));
                  temp1.insert(temp1.end(), temp2.begin(), temp2.end());
                  return fraction(temp1, multiply(denominator, b.denominator)).simplify();
              }

              bool algebraic_expression::fraction::operator==(const fraction& b) const {
                  return (*this+b*fraction(-1)).numerator.empty();
              }

              // purpose of this function is to extract rational number from fraction for purposes of evaluating a^b in term::simplify
              // returns false if rational number cannot be extracted
              // implementation now is really shabby: works only if fraction direct rational number; unknown if it isnt
              std::pair<bool, big_num_rational> algebraic_expression::fraction::extract_rational_number() const {
                  if ((numerator.size()>1) || (denominator.size()>1) || (denominator.empty()) || (!denominator.front().vars.index.empty()) ||
                       ((numerator.size()==1) && (!numerator.front().vars.index.empty()))) {
                      return std::make_pair(*this==fraction(0), 0);
                  }
                  if (numerator.empty()) {
                      return std::make_pair(*this==fraction(0), 0);
                  }
                  big_num_rational candidate(0);
                  for(const auto& t : denominator) {
                      if (term::same_vars(t, numerator.front())) {  candidate = (numerator.front()/t).coeff; break; }
                  }

                  return std::make_pair(*this==fraction( std::vector<term>{  term(candidate) }, std::vector<term>{ term(1) }), candidate);
              }

              std::pair<big_num_rational, big_num_rational> algebraic_expression::fraction::extract_factor(const std::string& name) const {
                  std::vector<big_num_rational> ret;
                  std::vector<const std::vector<term>*> numden{ &numerator, &denominator };
                  for(const auto& temp : numden) {
                      big_num_rational ans(10000005485400LL);
                      for(const auto& t : *temp) {
                          if (t.vars.index.count(name)>0) {
                              auto tt = t.vars.index.at(name).extract_rational_number();
                              if (tt.first) {
                                  ans = std::min(ans, tt.second);
                              }
                          } else {
                              ans = std::min(ans, big_num_rational(0));
                          }
                      }
                      ret.push_back(ans);
                  }
                  return std::make_pair(ret[0], ret[1]);
              }

              // cancel common divisors of all coeff's and common var powers
              algebraic_expression::fraction& algebraic_expression::fraction::cancel_common() {
                  check(!numerator.empty() && !denominator.empty()) << "cancel_common called with either empty num or denom\n";

                  term gcd_val(0);
                  for(const auto& t : numerator)   {    gcd_val.coeff = big_num_rational::gcd(gcd_val.coeff, t.coeff);  }
                  for(const auto& t : denominator) {    gcd_val.coeff = big_num_rational::gcd(gcd_val.coeff, t.coeff);  }

                  std::set<std::string> var_names_extract;
                  for(const auto& t : numerator)   { for(const auto& nomen : t.vars.index) { var_names_extract.insert(nomen.first); } }
                  for(const auto& t : denominator) { for(const auto& nomen : t.vars.index) { var_names_extract.insert(nomen.first); } }

                  for(const auto& nomen : var_names_extract) {
                      auto ext_pair = extract_factor(nomen);
                      auto ext_power = std::min(ext_pair.first, ext_pair.second);
                      if (ext_power<10000005485400LL) { gcd_val = gcd_val * term(nomen, fraction(ext_power));  }
                  }

                  for(auto& t : numerator)   {     t = t/gcd_val;      }
                  for(auto& t : denominator) {     t = t/gcd_val;      }

                  return *this;
              }

              // TODO: if numerator.size() and denominator.size() are 1 just divide terms and print; or consider finding gcd by dividing terms somehow;
              // in any case make sure the result of finding roots for 1+5x^3 produces nice result
              expression algebraic_expression::fraction::to_expression(const map_bf<fraction,int>& dep_var,
                                                                       const std::map<std::string, map_bf<fraction,int>>& dep_var_func) const {
                  fraction frac_copy(*this);
                  if ((frac_copy.numerator.empty()) && (!frac_copy.denominator.empty())) {
                      return expression(0LL);
                  }
                  if ((frac_copy.numerator.empty()) && (frac_copy.denominator.empty())) {
                      return expression(0LL)/expression(0LL);
                  }

                  if (!frac_copy.denominator.empty()) frac_copy.cancel_common();

                  if ((frac_copy.denominator.size()==1) && (frac_copy.denominator.front().coeff<0)) {
                      frac_copy.denominator.front().coeff = frac_copy.denominator.front().coeff * (-1);
                      for(auto& t : frac_copy.numerator) { t.coeff = t.coeff * (-1); }
                  }

                  expression num, denom;
                  num.root = new expression::eq_node(plus_func.name);
                  for(auto x : frac_copy.numerator) {
                      bool negate = x.coeff<0;
                      x.coeff = abs(x.coeff);
                      expression temp(x.to_expression(dep_var, dep_var_func));
                      if (negate) {
                        num.root->children.push_back(new expression::eq_node(minus_unary_func.name));
                        num.root->children.back()->children.push_back(temp.root);
                      } else {
                        num.root->children.push_back(temp.root);
                      }
                      temp.root = nullptr;
                  }
                  if (frac_copy.numerator.size()==1) {
                      expression::eq_node* temp = num.root;
                      num.root = temp->children.front();
                      delete temp;
                  }
                  if ((frac_copy.denominator.size()==1) && (frac_copy.denominator.front().coeff==1) && (frac_copy.denominator.front().vars.index.empty())) {
                      return num;
                  }
                  denom.root = new expression::eq_node(plus_func.name);
                  for(auto x : frac_copy.denominator) {
                      bool negate = x.coeff<0;
                      x.coeff = abs(x.coeff);
                      expression temp(x.to_expression(dep_var, dep_var_func));
                      if (negate) {
                        denom.root->children.push_back(new expression::eq_node(minus_unary_func.name));
                        denom.root->children.back()->children.push_back(temp.root);
                      } else {
                        denom.root->children.push_back(temp.root);
                      }
                      temp.root = nullptr;
                  }
                  if (frac_copy.denominator.size()==1) {
                      expression::eq_node* temp = denom.root;
                      denom.root = temp->children.front();
                      delete temp;
                  }
                  return num/denom;
              }

             
              // TODO: logic for blocking of expansion of large expressions should be here
              std::vector<algebraic_expression::term> algebraic_expression::fraction::multiply(const std::vector<term>& a, const std::vector<term>& b) {
                  std::vector<term> ret;
                  for(auto x : a) {
                      for(auto y : b) {
                          ret.push_back(x*y);
                      }
                  }
                  return ret;
              }

              // TODO: logic for blocking of expansion of large expressions should be here
              algebraic_expression::fraction& algebraic_expression::fraction::simplify() {
                  std::vector<term> num_new;
                  std::map<std::string, fraction> ln_temp;
                  for(const auto& t : numerator) {
                      if (t.has_one_ln()) {
                          std::string var_name = t.vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string)->first;
                          var_name = var_name.substr(2 + ln_func.name.size());
                          if (big_pos_int::is_num(var_name)) {
                             term t_prod(t);
                             t_prod.vars.index.erase(t_prod.vars.index.lower_bound(function_marker_string + ln_func.name + function_marker_string));
                             ln_temp[var_name] = ln_temp[var_name] + fraction(std::vector<term>{ t_prod }, std::vector<term>{ term(1) });
                          } else {
                             num_new.push_back(t);
                          }
                      } else {
                          num_new.push_back(t);
                      }
                  }
                  term::pairwise_prime(ln_temp);
                  for(const auto& f : ln_temp) {
                      for(auto temp : f.second.numerator) {
                        if (f.first != "1") {
                          temp = temp * term(function_marker_string+ln_func.name+function_marker_string+f.first, fraction(1));
                          num_new.push_back(temp);
                        }
                      }
                  }

                  numerator = move(num_new);

                  std::vector<std::vector<term>*> numden{ &numerator, &denominator };
                  for(int i = 0; i < 2; ++i) {
                      *numden[i] = simplify_sum(*numden[i]);
                  }
                  if ((denominator.size()==1) && !((denominator.front().coeff==1) && (denominator.front().vars.index.empty()))) {
                      for(auto& t : numerator) { t = t/denominator.front(); }
                      denominator.front() = term(1);
                  }
                  return *this;
              }

              std::vector<algebraic_expression::term> algebraic_expression::fraction::simplify_sum(const std::vector<term>& a) {
                  map_bf<term, big_num_rational> ret_(algebraic_expression::term::same_vars);
                  for(const auto& x : a) {
                    if (x.coeff!=0) {
                      auto it = ret_.find(x);
                      if (it == ret_.end()) {
                         ret_[x] = 1;
                      } else {
                         it->second = it->second + (x/it->first).coeff;
                      }
                    }
                  }
                  std::vector<term> ret;
                  for(const auto& x : ret_) {
                      if (x.second!=0) {
                          term temp(x.first); temp.coeff = temp.coeff * x.second;
                          ret.emplace_back(temp);
                      }
                  }
                  return ret;
              }

              algebraic_expression::fraction algebraic_expression::fraction::ln_of_term(const algebraic_expression::term& a) {
                  fraction ret(0);
                  if (a.coeff<0) {
                      ret = fraction(std::vector<term>{ term(imaginary_unit.name, fraction(1)) * term(pi_number.name, fraction(1)) }, 
                                     std::vector<term>{ term(1) });
                  }
                  std::map<std::string, fraction> temp(a.vars.index);
                  if (abs(a.coeff.numerator)>1) {    temp[abs(a.coeff.numerator).to_string()] = fraction(1);   }
                  if (abs(a.coeff.denominator)>1) {  temp[a.coeff.denominator.to_string()] = fraction(-1); }
                  term::pairwise_prime(temp);
                  for(auto f : temp) {
                      if (f.first != "1") {
                        if (f.first!=euler_number.name) {
                          f.second = f.second * fraction(std::vector<term>{ term(function_marker_string+ln_func.name+function_marker_string+f.first, fraction(1))  }, std::vector<term>{ term(1) });
                        }
                        ret = ret + f.second;
                      }
                  }
                  return ret;
              }


        // returns fraction of the expression ponted at by root; can only contain +,-,*,pow,tokens from dict and numbers; 

        algebraic_expression::fraction algebraic_expression::common_denominator(expression::eq_node* root) {
            term one(big_num(1));
            std::vector<fraction> arguments;
            for(const auto& f : root->children) {
                arguments.push_back(common_denominator(f));
            }
            check(((default_eval.known_functions_dict.count(root->token_name)==0) || 
                (root->children.size()==default_eval.known_functions_dict[root->token_name].args) || 
                (default_eval.known_functions_dict[root->token_name].args==-1))) 
                << "Function " << root->token_name << " has wrong number of arguments\n";
            check((default_eval.known_functions_dict.count(root->token_name) > 0) || (root->children.empty())) 
                << "Unknown function " << root->token_name <<  "\n";

            if ((default_eval.known_functions_dict.count(root->token_name)>0) && 
               ((root->children.size()==default_eval.known_functions_dict[root->token_name].args) || 
                                 (default_eval.known_functions_dict[root->token_name].args==-1))) {
                   auto attempt_evaluation = default_eval.known_functions_dict[root->token_name].eval_function(arguments);
                   if (attempt_evaluation.first) {
                       return attempt_evaluation.second;
                   }
            }
            if (root->token_name == power_func.name) {
              fraction& pow = arguments.back();
              fraction& base = arguments.front();
              auto is_negative_power = pow.extract_rational_number();
              check(!is_negative_power.first || (is_negative_power.second>0) || !(base==fraction(0))) << "0^negative expression encountered!\n";
              check(!base.denominator.empty()) << "denominator inexplicably empty\n";
              if (base.numerator.empty()) {  return fraction(0);   }

              std::vector<std::vector<term>*> numdenbase{ &base.numerator, &base.denominator  };
              std::vector<term> token_numden;
              for(int i = 0; i < 2; ++i) {
                  term temp(numdenbase[i]->front());
                  if (numdenbase[i]->size()>1) {
                    fraction num_temp = fraction(*numdenbase[i], std::vector<term>{ term(1) });
                    if (!dependent_vars.exists(num_temp)) {
                      int new_no = dependent_vars.size();
                      dependent_vars[num_temp] = new_no;
                    }
                    temp = term(dependent_start + std::to_string(dependent_vars[num_temp]), fraction(1));
                  }
                  token_numden.push_back(temp^pow);
              }
              return fraction(std::vector<term>{ token_numden[0] }, std::vector<term>{ token_numden[1] }).simplify();
            }
            if (root->token_name == ln_func.name) {
                fraction& arg = arguments.front();
                check(!arg.numerator.empty()) << "ln called on 0\n";
                check(!arg.denominator.empty()) << "argument of ln inexplicably empty!\n";
                std::vector<std::vector<term>*> numdenbase{ &arg.numerator, &arg.denominator  };
                std::vector<term> token_numden;
                for(int i = 0; i < 2; ++i) {
                  term temp(numdenbase[i]->front());
                  if (numdenbase[i]->size()>1) {
                    fraction num_temp = fraction(*numdenbase[i], std::vector<term>{ term(1) });
                    if (!dependent_vars.exists(num_temp)) {
                      int new_no = dependent_vars.size();
                      dependent_vars[num_temp] = new_no;
                    }
                    temp = term(dependent_start + std::to_string(dependent_vars[num_temp]), fraction(1));
                  }
                  token_numden.push_back(temp);
                }
                term ln_arg(token_numden[0]/token_numden[1]);
                return fraction::ln_of_term(ln_arg);
            }
            if (root->children.empty()) {
              if (big_num::is_num(root->token_name)) {
                return fraction(big_num(root->token_name));
              } else {
                return fraction(std::vector<term>{ term(root->token_name, fraction(1)) }, std::vector<term>{ one });
              }
            } else {
                if (func_arg.count(root->token_name)==0) {
                    func_arg[root->token_name] = map_bf<fraction, int>(default_eval.known_functions_dict[root->token_name].symmetry_equal);
                }
                map_bf<fraction,int>& depv = func_arg[root->token_name];
                if (!depv.exists(arguments.front())) {
                    int new_no = depv.size();
                    depv[arguments.front()] = new_no;
                }
                term ret_term(dependent_start_function + root->token_name + function_marker_string + std::to_string(depv[arguments.front()]), fraction(1));
                ret_term.coeff = default_eval.known_functions_dict[root->token_name].symmetry_coeff(arguments.front(), 
                                               depv.find(arguments.front())->first  );
                return fraction(std::vector<term>{ ret_term }, std::vector<term>{ one });
            }

            // control of program should never reach here, the following is here pro forma
            return fraction(1);
        }

        expression algebraic_expression::to_expression() {
            return value.to_expression(dependent_vars, func_arg);
        }

        // this expands integer powers of dependent variables; alse evaluates fractional powers of i and e^(2 pi q) where possible
        // TODO: improve this by first scanning all terms in num and den, for negative integer powers, then multiply by common fact to get rid of them
        // TODO: expansion of powers below 7 should be improved to use multinomial theorem
        // TODO: trick with level is a temporary fix until implementation of im and re parts finished; when they are the condition
        //       (*) below should have additional condition that im part of pow in e^pow isnt zero; now infinite recursion on expressions like e^2
        algebraic_expression::fraction algebraic_expression::expand(fraction frac, int level) {
              if ((frac.extract_rational_number().first) || (level==5)) { return frac; }
              std::vector<std::vector<term>*> numdenbase{ &frac.numerator, &frac.denominator  };
              std::vector<fraction> token_numden;
              for(int i = 0; i < 2; ++i) {
                  token_numden.push_back(fraction(0));
                  for(auto t : *numdenbase[i]) {
                    fraction term2(1);
                    auto it = t.vars.index.begin();
                    while (it != t.vars.index.end()) {
                      it->second = expand(it->second, level+1);
                      auto result = it->second.extract_rational_number();
                      if (!result.first) {  ++it;  continue;  }
                      if ((it->first.substr(0, dependent_start.size())==dependent_start) && (big_num_rational::is_int(result.second)) && (abs(result.second.numerator)<7)) {
                        for(big_pos_int i = 0; i < abs(result.second.numerator); i = i+1) {
                            if (result.second>0) { 
                                term2 = term2 * dependent_vars.at(stoi(it->first.substr(dependent_start.size())));
                            } else {
                                term2 = term2 / dependent_vars.at(stoi(it->first.substr(dependent_start.size())));
                            }
                        }
                        it = t.vars.index.erase(it);
                        term2 = expand(term2, level+1);
                        continue;
                      }
                      ++it;
                    }

                    // what follows is taking care of 2pi i periodicity and special values of e^(...)
                    // TODO: consider adding powers of all bases, not just e and i; for example 2^(2 pi i/ln 2)

                    fraction term3(1);
                    // (*)
                    if ((t.vars.index.count(euler_number.name)>0) || ((t.vars.index.count(imaginary_unit.name)>0) && 
                         !(t.vars.index[imaginary_unit.name]==fraction(0)) && !(t.vars.index[imaginary_unit.name]==fraction(1)) )) {
                      fraction tot_pow = t.vars.index[euler_number.name] + t.vars.index[imaginary_unit.name] * 
                                       fraction(std::vector<term>{ term(imaginary_unit.name, fraction(1)) * term(pi_number.name, fraction(1)) }, 
                                       std::vector<term>{  term(2) });

                      t.vars.index.erase(imaginary_unit.name); t.vars.index.erase(euler_number.name);
                      term3 = expand(default_eval.known_functions_dict[exp_func.name].eval_function(std::vector<fraction>{ tot_pow }).second, level+1);
                    }

                    token_numden.back() = token_numden.back() + term2 * fraction(std::vector<term>{ t }, std::vector<term>{ term(1) }) * term3;
                }
              }
              return token_numden[0]/token_numden[1];
        } 

      expression& expression::derivative(const std::string& var) {
          expression temp(derivative_helper(root, var).simplify_rational());
          expression to_delete; to_delete.root = root;
          root = temp.root;
          temp.root = nullptr;
          return *this;
      }

      expression expression::derivative_helper(eq_node* root, const std::string& var) {
          if (root->children.empty()) {
              if (root->token_name==var) {
                  return expression(new eq_node("1"));
              } else {
                  return expression(new eq_node("0"));
              }
          }
          check(default_eval.known_functions_dict.count(root->token_name)>0) << "unknown function " << root->token_name << "\n";
          expression ret;
          ret.root = new expression::eq_node(plus_func.name);
          for(int i=0; i < root->children.size(); ++i) {
              expression temp1(default_eval.known_functions_dict[root->token_name].derivative(root->children, i));
              expression temp2(derivative_helper(root->children[i], var));
              if ((temp1.root->token_name=="0") || (temp2.root->token_name=="0")) {
                  continue;
              }
              ret.root->children.push_back(new eq_node(multiply_func.name));
              ret.root->children.back()->children.push_back(temp1.root); temp1.root = nullptr;
              ret.root->children.back()->children.push_back(temp2.root); temp2.root = nullptr;             
          }
          if (ret.root->children.empty()) {
              return expression(new eq_node("0"));
          }
          return ret;
      }

      std::string expression::to_latex() const {
          expression expr_copy(*this);
          std::stringstream out;
          post_order_crawler(expr_copy.root, [](eq_node* root) {
              if ((root->token_name[0]=='-') && (root->children.empty())) {
                  root->children.push_back(new eq_node(root->token_name.substr(1)));
                  root->token_name = minus_unary_func.name;
              }
          });
          to_latex_helper(expr_copy.root, out, nullptr);
          std::string ret(out.str());
          if (ret.empty()) {
              return "0";
          }
          return out.str();
      }

      void expression::to_latex_helper(eq_node* root, std::stringstream& out, eq_node* root_pow) {
          if (root->token_name == "D") {
              out << "\\left(";
              to_latex_helper(root->children.front(), out, nullptr);
              out << "\\right)^{\\prime}";
              return;
          }
          if ((default_eval.known_functions_dict.count(root->token_name)>0) && (!default_eval.known_functions_dict[root->token_name].bookends.empty())) {
              for(int i = 0; i < root->children.size(); ++i) {
                  out << default_eval.known_functions_dict[root->token_name].bookends[i];
                  to_latex_helper(root->children[i], out, nullptr);
              }
              out << default_eval.known_functions_dict[root->token_name].bookends.back();
              return;
          }
          map_bf<std::string, int> addition_ops{ {plus_func.name, +1}, {minus_func.name, -1}, {minus_unary_func.name, -1} };
          if (root->children.empty()) {
              if (root->token_name==plus_func.name) {
                  out << "0";
              } else if (root->token_name==multiply_func.name) {
                  out << "1";
              } else {
                  out << root->token_name;
              }
              return;
          }
          if (addition_ops.exists(root->token_name)) {
              std::vector<std::pair<eq_node*,int>> summands;
              std::function<void(eq_node*,int)> crawl = [&summands,&crawl,&addition_ops](eq_node* root_, int sign) {
                  if (!addition_ops.exists(root_->token_name)) {
                    summands.push_back(std::make_pair(root_, sign));
                  }
                  if (root_->token_name==plus_func.name) {
                      for(const auto& ss : root_->children) {
                          crawl(ss, sign);
                      }
                  }
                  if (root_->token_name==minus_func.name) {
                      crawl(root_->children.front(), sign);
                      crawl(root_->children.back(), -sign);
                  }
                  if (root_->token_name==minus_unary_func.name) {
                      crawl(root_->children.front(), -sign);
                  }
              };
              crawl(root, +1);

              for(int i = 0; i < summands.size(); ++i) {
                  if (summands[i].second==-1) {
                      out << "-";
                  }
                  if ((summands[i].second==1) && (i>0)) {
                      out << "+";
                  }
                  to_latex_helper(summands[i].first, out, nullptr);
              }
              return;
          }

          if (root->token_name==multiply_func.name) {
              std::vector<eq_node*> multiplicants;
              std::function<void(eq_node*)> crawl = [&multiplicants,&crawl](eq_node* root_) {
                  if (root_->token_name!=multiply_func.name) {
                    multiplicants.push_back(root_);
                  }
                  if (root_->token_name==multiply_func.name) {
                      for(const auto& ss : root_->children) {
                          crawl(ss);
                      }
                  }
              };
              crawl(root);

              for(int i = 0; i < multiplicants.size(); ++i) {
                  if (i>0) {
                      out << "\\cdot ";
                  }
                  if ((multiplicants[i]->token_name[0]=='-') || (addition_ops.exists(multiplicants[i]->token_name))) {
                      out << "\\left(";
                  }
                  to_latex_helper(multiplicants[i], out, nullptr);
                  if ((multiplicants[i]->token_name[0]=='-') || (addition_ops.exists(multiplicants[i]->token_name))) {
                      out << "\\right)";
                  }
              }
              return;
          }

          if (root->token_name==power_func.name) {
                  if (root->children.front()->token_name==abs_func_name) {
                      to_latex_helper(root->children.front(), out, nullptr);
                      out << "^{";
                      to_latex_helper(root->children.back(), out, nullptr);
                      out << "}";
                      return;
                  }
                  bool special_power = ((root->children.front()->children.size()>0) && 
                     (root->children.front()->token_name!=id_func.name) && 
                     (root->children.front()->token_name!=plus_func.name) && 
                     (root->children.front()->token_name!=minus_func.name) &&
                     (root->children.front()->token_name!=multiply_func.name) && 
                     (root->children.front()->token_name!=divide_func.name) && 
                     (root->children.front()->token_name!=minus_unary_func.name) &&
                     (root->children.front()->token_name!=power_func.name) && 
                     (root->children.front()->token_name!=sqrt_func.name) && 
                     (root->children.front()->token_name!=sqrt_unary_func.name));
                  if (special_power) {
                      to_latex_helper(root->children.front(), out, root->children.back());
                  } else {
                    if ((!root->children.front()->children.empty()) || (root->children.front()->token_name[0]=='-')) {
                      out << "\\left(";
                    }
                    to_latex_helper(root->children.front(), out, nullptr);
                    if ((!root->children.front()->children.empty()) || (root->children.front()->token_name[0]=='-')) {
                      out << "\\right)";
                    }
                    out << "^{";
                    to_latex_helper(root->children.back(), out, nullptr);
                    out << "}";
                  }
                  return;
          }

          out << "{\\rm " << root->token_name << "}";
          if (root_pow!=nullptr) {
              out << "^{";
              to_latex_helper(root_pow, out, nullptr);
              out << "}";
          }
          if ((root->children.size()==1) && ((root->children.front()->token_name==abs_func.name) || (root->children.front()->token_name==divide_func.name)
                || (root->children.front()->token_name==power_func.name) || (root->children.front()->children.empty()))) {
            out << "\\,";
            to_latex_helper(root->children.front(), out, nullptr);
          } else {
            out << "\\left(";
            for(int i = 0; i < root->children.size(); ++i) {
                  to_latex_helper(root->children[i], out, nullptr);
                  if (i < root->children.size()-1) out << ",";
            }
            out << "\\right)";
          }
          return;
      }

      bool expression::dependent_on(const std::string& v) const {
          auto vars = get_tokens()[0];
          return (std::find(vars.begin(), vars.end(), v) != vars.end());
      }

      // returns if all functions with one or more args are recognized; if not the sec and third component have the name of unknown function
      // and its number of arguments
      std::tuple<bool,std::string,int> expression::is_valid() const {
          auto vars = get_tokens();
          for(const auto& name : vars[0]) {
              if ((default_eval.known_functions_dict.count(name)>0) && (default_eval.known_functions_dict[name].args!=0)) {
                  return std::make_tuple(false, name, 0);
              }
          }
          for(int i = 1; i < vars.size(); ++i) {
              for(const auto& name : vars[i]) {
                if ((default_eval.known_functions_dict.count(name)==0) || 
                   ((default_eval.known_functions_dict[name].args>=0) && (default_eval.known_functions_dict[name].args!=i))  ) {
                    return std::make_tuple(false, name, i);
                }
              }
          }
          return std::make_tuple(true, "", 0);
      }

      std::vector<expression> expression::derivative_steps(const std::string& var) {
          std::vector<expression> ret;
          ret.push_back(*this);
          eq_node* temp = new eq_node("D");
          temp->children.push_back(ret.back().root);
          ret.back().root = temp;

          while (true) {
              std::vector<eq_node*> der;
              expression new_step(ret.back());
              post_order_crawler(new_step.root, [&der](eq_node* root){
                  if (root->token_name=="D") {
                      der.push_back(root);
                  }
              });
              if (der.empty()) { break; }
              for(const auto& root_ : der) {
                  eq_node* root = root_->children.front();
                  map_bf<std::string, int> addition_ops{ {plus_func.name, +1}, {minus_func.name, -1}, {minus_unary_func.name, -1} };

                    // addition/subtraction handled separately
                  if (addition_ops.exists(root->token_name)) {
                      std::vector<eq_node*> summands;
                      std::function<void(eq_node*)> crawl = [&summands,&crawl,&addition_ops](eq_node* root_) {
                        if (!addition_ops.exists(root_->token_name)) {
                          summands.push_back(root_);
                        }
                        if (root_->token_name==plus_func.name) {
                          for(const auto& ss : root_->children) {
                            crawl(ss);
                          }
                        }
                        if (root_->token_name==minus_func.name) {
                          crawl(root_->children.front());
                          crawl(root_->children.back());
                        }
                        if (root_->token_name==minus_unary_func.name) {
                          crawl(root_->children.front());
                        }
                      };
                      crawl(root);
                      
                      for(auto t : summands) {
                          eq_node* copy_node = new eq_node(*t);
                          t->token_name = "D";
                          t->children = std::vector<eq_node*>{copy_node};
                      }

                      *root_ = *root;
                      delete root;
                      continue;
                  }

                  expression new_r;
                  if (root->children.empty()) {
                    if (root->token_name==var) {
                      new_r = expression(new eq_node("1"));
                    } else {
                      new_r = expression(new eq_node("0"));
                    }
                  } else {
                    check(default_eval.known_functions_dict.count(root->token_name)>0) << "unknown function " << root->token_name << "\n";
                    new_r.root = new expression::eq_node(plus_func.name);
                    for(int i=0; i < root->children.size(); ++i) {
                        expression temp1(default_eval.known_functions_dict[root->token_name].derivative(root->children, i));
                        if ((root->children[i]->token_name==var) && (root->children[i]->children.empty())) {
                            new_r.root->children.push_back(temp1.root); temp1.root = nullptr; continue;
                        }
                        expression temp2(root->children[i]);
                        eq_node* temp_node = new eq_node("D");
                        temp_node->children.push_back(temp2.root);
                        temp2.root = temp_node;
                        if (!expression(root->children[i]).dependent_on("x")) { continue; }
                        if (!expression(temp2.root->children.front()).derivative("x").dependent_on("x")) {
                            temp2 = expression(temp2.root->children.front()).derivative("x");
                        }
                        if (temp1.root->token_name=="1") {
                          new_r.root->children.push_back(temp2.root); temp2.root = nullptr;
                        } else {
                          new_r.root->children.push_back(new eq_node(multiply_func.name));
                          new_r.root->children.back()->children.push_back(temp1.root); temp1.root = nullptr;
                          new_r.root->children.back()->children.push_back(temp2.root); temp2.root = nullptr;
                        }
                    }
                  }
                  *root_ = *new_r.root;
                  new_r.root = nullptr;
              }
              ret.push_back(new_step);
          }

          return ret;
      }


      // does expression conatain only numbers ?
      bool expression::no_vars() const {
          bool ret = true;
          auto temp = get_tokens()[0];
          for(const auto& v : temp) {
              ret &= ((big_num::is_num(v)) || (v==euler_number.name) || (v==pi_number.name) || (v==imaginary_unit.name));
          }
          return ret;
      }

      bool graph::can_draw_graph(expression expr, const std::string& var) {
          expr.substitute(var, expression(0LL));
          return expr.no_vars();
      }

      std::vector<std::pair<double,double>> graph::get_pts(const expression& expr, const std::string& var, 
         double xfrom, double xto, double yfrom, double yto, int n) {
          check(can_draw_graph(expr, var) && (n>1)) << "graph can't be drawn\n";
          std::vector<std::pair<double,double>> ret;
          for(int i = 0; i < n; ++i) {
              double x = xfrom + (xto-xfrom)*i/(n-1);
              cd val = default_eval.evaluate(expr, {{var, cd(x)}});
              if (std::isfinite(val.real()) && (std::isfinite(val.imag())) && (std::abs(val.imag()) < 0.000001) 
                  && (val.real()>=yfrom) && (val.real()<=yto)) {
                  ret.push_back(std::make_pair(x, val.real()));
              }
          }
          return ret;
      }

      std::vector<std::tuple<double,double,double>> graph::stat(const expression& expr, const std::string& var, int N) {
          std::vector<std::tuple<double,double,double>> ret;
          expression expr_der(expr); expr_der.derivative(var);
          for(long long x1 = 1; x1 <= 100000; x1 *= 10) {
            for(long long x2 : std::vector<long long>{ 1, 2, 5 }) {
              long long x = x1 * x2;
              auto pts = get_pts_tempered(expr, var, -x, x, -x, x, N, expr_der);
              if (pts.size()>1) {
                  double nn = 0;
                  double s = 0, s2 = 0;
                  for(int i = 0; i < pts.size()-1; ++i) {
                      auto x = pts[i];
                      double w = (std::get<2>(x) ? 0 : std::get<0>(pts[i+1])-std::get<0>(x));
                      s += std::get<1>(x)*w; s2 += std::get<1>(x)*std::get<1>(x)*w;
                      nn += w;
                  }
                  double average = s/nn;
                  double sigma = sqrt(s2/nn-average*average);
                  if (std::isfinite(sigma)) {
                     ret.push_back(std::make_tuple(sigma/x,1.0*pts.size()/100, x));
                  }
              }
            }
          }
          return ret;
      }

      std::vector<std::tuple<double,double,bool>> graph::get_pts_tempered(const expression& expr, const std::string& var, double xfrom, double xto, 
         double yfrom, double yto, int n, const expression& expr_der) {
          check(can_draw_graph(expr, var) && (n>1)) << "graph can't be drawn\n";

          std::vector<std::vector<std::pair<double, double>>> ret;
          ret.push_back(std::vector<std::pair<double,double>>());
          double xdelta = (xto-xfrom)/n, x(xfrom);
          // number of pts needed to advance by delta
          const int N = 50;
          while (x<xto) {
              cd val = default_eval.evaluate(expr, {{var, cd(x)}});
              cd vald = default_eval.evaluate(expr_der, {{var, cd(x)}});
              if ((std::isfinite(val.real())) && (std::isfinite(val.imag())) && (std::abs(val.imag()) < 0.000001) &&
                  (val.real()>=yfrom) && (val.real()<=yto) &&
                  (std::isfinite(vald.real())) && (std::isfinite(vald.imag())) && (std::abs(vald.imag()) < 0.000001) &&
                  ((ret.back().size()<N) || (ret.back()[ret.back().size()-N].first<x-xdelta))) {
                      ret.back().push_back(std::make_pair(x, val.real()));
                      x += xdelta/sqrt(1+vald.real()*vald.real());
              } else {
                  x += xdelta;
                  if (!ret.back().empty()) {
                      ret.push_back(std::vector<std::pair<double,double>>());
                  }
              }
          }
          std::vector<std::tuple<double,double,bool>> ans;
          for(const auto& row : ret) {
              if ((!row.empty()) && ((row.back().first-row.front().first)/(xto-xfrom) > 0.05)) {
                  for(const auto& pt : row) {
                      ans.push_back(std::make_tuple(pt.first, pt.second, false));
                  }
                  std::get<2>(ans.back()) = true;
              }
          }
          return ans;
      }

      std::pair<std::vector<expression>, std::vector<expression>> polynomial::divmod(const std::vector<expression>& coeff, const std::vector<expression>& coeffb) {
          if (coeff.size()<coeffb.size()) {    return std::make_pair(std::vector<expression>{expression(0LL)}, coeff);      }
          std::vector<expression> q(std::vector<expression>((int)coeff.size()-(int)coeffb.size()+1, expression(1)));
          std::vector<expression> copy_(coeff);
          int max_deg = coeffb.size()-1;
          int curr = coeff.size()-1;
          expression lead_inv = expression(1)/coeffb[max_deg];
          while (curr >= max_deg) {
              q[curr-max_deg] = (copy_[curr] * lead_inv);
              for(int i = curr; i-curr+max_deg>=0; --i) {
                  copy_[i] = copy_[i] - (q[curr-max_deg] * coeffb[i-curr+max_deg]);
              }
              --curr;
          }
          while ((q.size()>1) && (q.back()==expression(0LL))) {
              q.pop_back();
          }
          while ((copy_.size()>1) && (copy_.back()==expression(0LL))) {
              copy_.pop_back();
          }
          return make_pair(q, copy_);
      }

    std::vector<expression> polynomial::gcd(const std::vector<expression>& a, const std::vector<expression>& b, int level) {
        if (level==0) {
            return std::vector<expression>{expression(1)};
        }
        if (b.empty() || ((b.size()==1) && (b.back()==expression(0LL)))) {
            return a;
        }
        return gcd(b, divmod(a,b).second, level-1);
    }

    expression expression::apply(const std::string& token, std::vector<expression> args) {
        expression ret;
        ret.root = new eq_node(token);
        for(auto& a : args) {
          ret.root->children.push_back(a.root);
          a.root = nullptr;
        }
        return ret;
    }

    expression polynomial::to_expression(const std::vector<expression>& coeff, const std::string& var_name) {
       std::vector<expression> summands;
       for(int i=0; i < coeff.size(); ++i) {
        if (!(coeff[i]==expression(0LL))) {
            switch (i) {
              case 0:   summands.push_back(coeff[i]); break;
              case 1:   summands.push_back(coeff[i] * expression(var_name)); break;
              default:  summands.push_back(coeff[i] * expression(var_name+"^"+std::to_string(i))); break;
            }
        }
       }
       return expression::apply(plus_func_name, summands);
    }


      std::vector<big_num_rational> polynomial::rational_zeros(const std::vector<big_num_rational>& coeff) {

          std::function<big_num_rational(const big_num_rational&)> eval = [&coeff](const big_num_rational& x) {
              big_num_rational ret = 0;
              big_num_rational x_curr = 1;
              for(const auto& c : coeff) {
                  ret = ret + c * x_curr;
                  x_curr = x_curr * x;
              }
              return ret;
          };
          int first_nonzero;

          if (eval(0)==0) {
              return std::vector<big_num_rational>{0};
          }

          big_pos_int lcm = coeff.front().denominator;
          for(const auto& den : coeff) {
              lcm = lcm * den.denominator / big_pos_int::gcd(lcm, den.denominator);
          }

          auto div1 = divisors(abs(coeff.front() * big_num_rational(lcm,1)).numerator);
          auto div2 = divisors(abs(coeff.back() * big_num_rational(lcm,1)).numerator);
          std::set<big_num_rational> solutions;

          for(const auto& n : div1) {
              for(const auto& d : div2) {
                  auto num = big_num_rational(n,d);
                  if (eval(num)==0) { solutions.insert(num); }
                  if (eval(-num)==0) { solutions.insert(-num); }
              }
          }
          return std::vector<big_num_rational>(solutions.begin(), solutions.end());
      }

     std::vector<expression> polynomial::zeros(std::vector<expression> coeff, bool only_first) {
          while ((!coeff.empty()) && (coeff.back()==expression(0LL))) {
              coeff.pop_back();
          }
          check(!coeff.empty()) << "zeros called on zero polynomial!!\n";
          if (coeff.size()==1) { return std::vector<expression>(); }
          std::vector<expression> ret;
          auto rat_coeff(rational(coeff));
          if (!rat_coeff.empty()) {
              // has to be a loop to catch possible multiple zeros
              while (true) {
                auto rat_zeros(rational_zeros(rat_coeff));
                if (rat_zeros.empty()) { break; }
                for(const auto& z : rat_zeros) {
                    ret.push_back((expression(z.numerator.to_string())/expression(z.denominator.to_string())).simplify_rational());
                    std::vector<big_num_rational> new_coeff(rat_coeff.size()-1, 0);
                    for(int i = 1; i < rat_coeff.size(); ++i) {
                        big_num_rational c = rat_coeff[i];
                        for(int j = i-1; j >= 0; --j) {
                            new_coeff[j] = new_coeff[j] + c;
                            c = c * z;
                        }
                    }
                    rat_coeff = std::move(new_coeff);
                }
              }
              coeff.clear();
              for(const auto& x : rat_coeff) {
                  coeff.push_back(expression(x.numerator.to_string())/expression(x.denominator.to_string()));
              }
          }
          if (coeff.size()==2) {
              expression a = coeff.back(), b = coeff.front();
              ret.push_back((-b/a));
          }
          if (coeff.size()==3) {
              expression a = coeff.back(), b = coeff[1], c = coeff.front();
              expression sqrtD = ((b^expression(2))-expression(4)*a*c)^(expression(1)/expression(2));
              ret.push_back(((-b-sqrtD)/(expression(2)*a)));
              ret.push_back(((-b+sqrtD)/(expression(2)*a)));
          }
          if (coeff.size()==4) {
              expression b = coeff[2]/coeff.back(), c = coeff[1]/coeff.back(), d = coeff.front()/coeff.back();
              expression p = c - (b^expression(2))/expression(3), q = (expression(2)*(b^expression(3))-expression(9)*b*c+expression(27)*d)/(expression(27));
              expression sqrtD = (q*q/expression(4) + p*p*p/expression(27)); sqrtD = sqrtD^(expression(1)/expression(2));
              expression alfa = (-q/expression(2)+sqrtD); alfa = alfa^(expression(1)/expression(3));
              expression beta = -p/(expression(3)*alfa);
              if (p==expression(0LL)) {
                  alfa = expression(0LL);
                  beta = -expression::apply(sqrt_func_name, {expression(3), q});
              }
              ret.push_back(( -b/expression(3) + alfa + beta));
              expression root_plus = (-expression(1)+expression(imaginary_unit.name+"*\\3"))/expression(2);
              expression root_minus = (-expression(1)-expression(imaginary_unit.name+"*\\3"))/expression(2);
              ret.push_back(  ( -b/expression(3) + root_minus * alfa + root_plus * beta));
              ret.push_back(  ( -b/expression(3) + root_plus * alfa + root_minus * beta));
          }
          if (coeff.size()==5) {
              expression b_ = coeff[3]/coeff.back(), c_ = coeff[2]/coeff.back(), d_ = coeff[1]/coeff.back(), e_ = coeff.front()/coeff.back();
              expression c = c_ - expression(3)*b_*b_/expression(8);
              expression d = d_ - b_*c_/expression(2) + b_*b_*b_/expression(8);
              expression e = e_ - b_*d_/expression(4) + c_*b_*b_/expression(16) - expression(3)*b_*b_*b_*b_/expression(256);

              if (d==expression(0LL)) {
                  for(auto ex : zeros(std::vector<expression>{ e, c, expression(1) })) {
                      ret.push_back( (-b_/expression(4) + ex^(expression(1)/expression(2))) );
                      ret.push_back( (-b_/expression(4) - (ex^(expression(1)/expression(2)))) );
                  }
              } else {
                expression p2 = zeros(std::vector<expression>{
                  -d*d, c*c - expression(4)*e, expression(2)*c, expression(1)
                }, true).front();
                expression p = p2^(expression(1)/expression(2));
                expression r = -p;
                expression s = (c+p2+d/p)/expression(2);
                expression q = (c+p2-d/p)/expression(2);
                for(const auto& ex : zeros(std::vector<expression>{ q,p,expression(1) })) {
                  ret.push_back((-b_/expression(4) + ex).simplify_rational());
                }
                for(const auto& ex : zeros(std::vector<expression>{ s,r,expression(1) })) { 
                  ret.push_back((-b_/expression(4) + ex).simplify_rational());
                }
              }
          }
          if (!ret.empty()) {
              algebraic_expression alg_exp(ret.front());
              if (alg_exp.is_ok) { ret.front() = alg_exp.to_expression(); }
              if (only_first) { return ret; }
              if (alg_exp.is_ok) {
                  for(int i = 1; i < ret.size(); ++i) {
                      ret[i].simplify_rational();
                  }
              }
          }
          return ret;
      }


      // returns empty vector if given poly is not in Q[x]; otherwise returns coefficients
      std::vector<big_num_rational> polynomial::rational(const std::vector<expression>& coeff) {
          std::vector<big_num_rational> ret;
          for(const auto& x : coeff) {
              algebraic_expression temp(x);
              if (temp.is_ok) {
                auto q = temp.value.extract_rational_number();
                if (q.first) {
                  ret.push_back(q.second);
                } else {
                  break;
                }
              }
          }
          if (ret.size()==coeff.size()) {
              return ret;
          } else {
              return std::vector<big_num_rational>();
          }
      }

    expression_const::expression_const() {
        _binary_operators = std::vector<std::vector<std::pair<char, std::string>>>{
           {  std::make_pair('+', plus_func_name), std::make_pair('-', minus_func_name) },   // lowest precedence
           {  std::make_pair('*', multiply_func_name), std::make_pair('/', divide_func_name)},
           {  std::make_pair('^', power_func_name), std::make_pair('\\', sqrt_func_name)  }    // highest precedence
        };

        _unary_operators = std::vector<std::pair<char, std::string>>{
           std::make_pair('-', minus_unary_func_name), std::make_pair('\\', sqrt_unary_func_name)
        };

        _token_alphabet = std::set<char>{'_','a','b','c','d','e','f','g','h','i','j','k','l','m','n',
                                               'o','p','q','r','s','t','u','v','w','x','y','z',
                                               'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
                                               'O','P','Q','R','S','T','U','V','W','X','Y','Z',
                                               '0','1','2','3','4','5','6','7','8','9', 
                                               '.'};

         _error_message = std::map<expression::error_handling::error_code, std::string>{ 
                                             { expression::error_handling::ok, "OK" }, 
                                             { expression::error_handling::empty, "Empty expression" }, 
                                             { expression::error_handling::disallowed_char, "Character not allowed" },
                                             { expression::error_handling::token_expected, "Expression expected"},
                                             { expression::error_handling::stray_comma, "Misplaced comma" },
                                             { expression::error_handling::ends_too_soon, "Expression ends too soon" },
                                             { expression::error_handling::redundant_closing_bracket, "Redundant closing bracket" }};

    }

    expression_const& expression_const::get_instance() {
        static expression_const inst;
        return inst;
    }

    std::vector<big_pos_int> polynomial::divisors(const big_pos_int& x) {
      if (x==1) { return std::vector<big_pos_int>{ 1 }; }
      if (x>erat.N) { return std::vector<big_pos_int>{1,x}; }
      long long val = stoll(x.to_string());
      long long smallest_prime = erat.lp[val];
      int k = 0;
      while (erat.lp[val]==smallest_prime) { val = val/smallest_prime; ++k; }
      std::vector<big_pos_int> ret;
      for(auto x : divisors(val)) {
        long long fact = 1;
        for(int i = 0; i <= k; ++i) {
            ret.push_back(x*fact);
            fact *= smallest_prime;
        }
      }
      return ret;
    }

    std::vector<expression> polynomial::derivative(std::vector<expression> coeff) {
        if (coeff.empty()) { return coeff; }
        coeff.erase(coeff.begin());
        for(int i = 0; i < coeff.size(); ++i) {
            coeff[i] = coeff[i] * expression(i+1);
        }
        return coeff;
    }

      // returns make_pair(quotient, partial fraction of remainder); partial fraction given as vector of make_tuple(zero, residue, power)
    std::pair<std::vector<expression>, std::vector<std::tuple<expression,expression,int>>> polynomial::partial_fractions(
                      std::vector<expression> num, const std::vector<expression>& denom, const std::vector<expression>& denom_zeros) {
        map_bf<expression, int> multiplicity;
        for(const auto& z : denom_zeros) {
            ++multiplicity[z];
        }
        std::pair<std::vector<expression>,std::vector<std::tuple<expression,expression,int>>> ret;
        auto dm = polynomial::divmod(num, denom);
        ret.first = dm.first;
        num = dm.second;
        for(const auto& z : multiplicity) {
            std::vector<expression> num_(2*z.second-1, expression(0LL)), denom_(z.second, expression(0LL));
            std::vector<expression> num_copy(num), denom_copy(denom);
            long long factorial = 1, factorial_denom = 1;
            for(int i = 0; i < z.second; ++i) {
                factorial_denom *= i+1;
                denom_copy = polynomial::derivative(denom_copy);
            }
            for(int i = 0; i < z.second; ++i) {
                num_[2*z.second-2-i] = polynomial::to_expression(num_copy, "x").substitute("x", z.first)/expression(factorial);
                num_copy = polynomial::derivative(num_copy);
                factorial *= i+1;

                denom_[z.second-1-i] = polynomial::to_expression(denom_copy, "x").substitute("x", z.first)/expression(factorial_denom);
                denom_copy = polynomial::derivative(denom_copy);
                factorial_denom *= z.second+i+1;
            }
            auto res = polynomial::divmod(num_, denom_).first;
            for(int i = 0; i < res.size(); ++i) {
                if (!(res[i]==expression(0LL))) ret.second.push_back(std::make_tuple(z.first, res[i], 1+i));
            }
        }        
        return ret;
    }

    std::vector<expression> polynomial::multiply(const std::vector<expression>& a, const std::vector<expression>& b) {
        std::vector<expression> ret(a.size() + b.size() - 1, expression(0LL));
        for(int i = 0; i < a.size(); ++i) {
            for(int j = 0; j < b.size(); ++j) {
                ret[i+j] = ret[i+j] + a[i] * b[j];
            }
        }
        return ret;
    }

    std::vector<expression> polynomial::sum(const std::vector<expression>& a, const std::vector<expression>& b) {
        std::vector<expression> ret(std::max(a.size(), b.size()));
        for(int i = 0; i < ret.size(); ++i) {
            ret[i] = (i < a.size() ? a[i] : expression(0LL)) + (i < b.size() ? b[i] : expression(0LL));
        }
        return ret;
    }

    std::vector<expression> polynomial::minus(const std::vector<expression>& a, const std::vector<expression>& b) {
        return polynomial::sum(a, polynomial::multiply({expression(-1)}, b));
    }
    
    void polynomial::simplify(std::vector<expression>& a) {
        for(auto& c : a) {
            c.simplify_rational();
        }
    }

    expression graph::graph_appropriate_expression(expression exp) {
        expression::post_order_crawler(exp.root, [](expression::eq_node* root) {
            if ((root->token_name==power_func_name) || (root->token_name==sqrt_func_name)) {
                expression pow;
                if (root->token_name==power_func_name) {
                    pow = expression(root->children.back());
                } else {
                    pow = expression(1LL)/expression(root->children.front());
                }
                // check pow a/b, a,b num, b odd
                auto pow_rat = pow.extract_rational_number();
                if ((pow_rat.first) && (pow_rat.second.denominator%2==1)) {
                    expression base;
                    if (root->token_name==power_func_name) {
                        base = expression(root->children.front());
                    } else {
                        base = expression(root->children.back());
                    }
                    expression b1 = base^expression(pow_rat.second.numerator.to_string());
                    expression pw1 = expression(1)/expression(pow_rat.second.denominator.to_string());
                    base = (expression::apply(abs_func_name, { b1 })^pw1) * b1/expression::apply(abs_func_name, { b1 });
                    expression del1, del2;
                    del1.root = root->children.front(); del2.root = root->children.back();
                    *root = *base.root;
                    base.root->children.clear();
                }
            }
        });
        return exp;
    }

    std::pair<bool, big_num_rational> expression::extract_rational_number() const {
        std::pair<bool, big_num_rational> ret(false, 0);
        if (no_vars()) {
            cd val = default_eval.evaluate(*this, {});
            if (std::isfinite(val.real()) && (std::isfinite(val.imag())) && (std::abs(val.imag()) < 0.000001)) {
              for(int i = 1; i < 100; ++i) {
                  if (std::abs( val.real()*i - std::round(val.real()*i) )<0.0000001) {
                      return std::make_pair(true, big_num_rational(std::llround(val.real()*i), i));
                  }
              }
            }
        }
        return ret;
    }
