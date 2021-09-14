
#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <functional>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <memory>

#include "bignum.hpp"

class expression {
    public:
      struct eq_node {
          std::vector<eq_node*> children;
          std::string token_name;
          eq_node(const std::string &);
      };

      struct error_handling {
        enum error_code { ok, empty, disallowed_char, token_expected, stray_comma, ends_too_soon, redundant_closing_bracket };
        error_code error;
        int position_of_error;

        error_handling(error_code);
      };

      eq_node* root;
      error_handling parsing_error;

      expression();

      expression(const std::string&);

      explicit expression(long long);

      explicit expression(eq_node*);

      expression(const expression&);

      expression(expression&&);

      expression operator/(const expression&) const;

      expression& operator=(expression&&);

      expression& operator=(const expression&);

      ~expression();

      bool operator==(const expression&) const;

      expression& parse(std::string, const std::vector<std::string>& = {});

      expression& substitute(const std::string&, const expression&);

      std::vector<std::vector<std::string>> get_tokens() const;

      expression operator+(const expression&) const;

      expression operator-(const expression&) const;

      expression operator*(const expression&) const;

      expression operator-() const;

      expression operator^(const expression&) const;

      // simplifies expression, if cancel_var true, divides num with denom if both polynomials in cancel_var
      expression& simplify_rational(const std::string& = "");

      expression& derivative(const std::string&);

      std::vector<expression> derivative_steps(const std::string&);

      std::string to_latex() const;

      bool dependent_on(const std::string&) const;

      bool no_vars() const;

      std::tuple<bool,std::string,int> is_valid() const;

      void print() const;

      void print_raw() const;

      static void post_order_crawler(eq_node*, const std::function<void(eq_node*)>&);

      static expression apply(const std::string&, std::vector<expression>);

      std::pair<bool, big_num_rational> extract_rational_number() const;

      private:

        static void substitute_helper(eq_node* root, const std::string&, const expression&);
        static void print_helper(eq_node*);
        static void print_helper_raw(eq_node*);
        static void delete_identity(eq_node*);
        static void delete_node(eq_node*);
        static eq_node* copy_subtree(eq_node*);
        static bool is_allowed_char(char);
        static bool is_token_char(char);
        static bool is_binary_operator(char);
        static bool is_unary_operator(char);
        static bool is_binary_operator_helper(char, std::vector<std::pair<char, std::string>>&);
        static expression derivative_helper(eq_node*, const std::string&);
        static void to_latex_helper(eq_node*, std::stringstream&, eq_node*);

        static std::string function_name(char, const std::vector<std::pair<char, std::string>>&);

        static std::list<std::pair<char,int>>::iterator modify_expression(std::list<std::pair<char,int>>&, std::list<std::pair<char,int>>::iterator, 
                                               std::stack<std::tuple<std::list<std::pair<char,int>>::iterator, std::list<std::pair<char,int>>::iterator,int>>&, 
                                               const std::vector<std::pair<char,std::string>>&);

};

class algebraic_expression {
    public:
        static const std::string dependent_start;
        static const std::string dependent_start_function;
        static const char function_marker;
        static const std::string function_marker_string;

        class term;

        class fraction {
            public:
              std::vector<term> numerator, denominator;

              fraction();

              fraction(const std::vector<term>&, const std::vector<term>&);

              explicit fraction(const big_num_rational&);

              fraction operator*(const fraction&) const;

              fraction operator/(const fraction&) const;

              fraction operator+(const fraction&) const;

              bool operator==(const fraction&) const;

              bool operator<(const fraction&) const;

              std::pair<bool, big_num_rational> extract_rational_number() const;

              fraction& cancel_common();

              expression to_expression(const map_bf<fraction,int>&, const std::map<std::string, map_bf<fraction,int>>&) const;

              fraction& simplify();

              static fraction ln_of_term(const term&);

              std::pair<big_num_rational, big_num_rational> extract_factor(const std::string&) const;

            private:

              static std::vector<term> multiply(const std::vector<term>&, const std::vector<term>&);

              static std::vector<term> simplify_sum(const std::vector<term>&);
        };

        class multiindex {
            public:
              std::map<std::string,fraction> index;
             
              multiindex() = default;

              multiindex(const std::map<std::string, fraction>&);

              multiindex operator+(const multiindex&) const;

              multiindex& simplify();

        };

        class term {
            public:
              big_num_rational coeff;
              multiindex vars;

              term() = delete;

              term(const multiindex&, const big_num_rational&);

              explicit term(const big_num_rational&);
              
              term(std::string, const fraction&);

              term(const big_pos_int&, const fraction&);

              term operator*(const term&) const;

              term operator/(term) const;

              bool operator<(const term&) const;

              bool operator==(const term&) const;

              term operator^(const fraction&) const;

              static bool same_vars(const term&, const term&);

              term& simplify();

              expression to_expression(const map_bf<fraction,int>&, const std::map<std::string, map_bf<fraction,int>>&) const;

              static void pairwise_prime(std::map<std::string, fraction>&);

              bool has_one_ln() const;

              bool has_no_ln() const;

            private:
              static std::vector<big_pos_int> factors(const std::vector<big_pos_int>&);

        };

        fraction value;
        // dependent variables should all be of form something/1, ie denominator in fraction should have one term equal to 1
        map_bf<fraction,int> dependent_vars;
        // argumants of functions; can be any fraction, not like one line above with denominator 1
        std::map<std::string,map_bf<fraction,int>> func_arg;
        // signals if constructor was successful
        bool is_ok;
        // num, denom coefficients of polynomial in var passed to constructor; if num,denom arent polynomials or are of high degree, empty
        std::vector<fraction> num_coeff, denom_coeff;

        algebraic_expression() = delete;

        algebraic_expression(expression, const std::string& = "", int = 3);

        fraction common_denominator(expression::eq_node*);

        expression to_expression();

        fraction expand(fraction,int);

};

// graph drawing stuff
class graph {
    public:
      static bool can_draw_graph(expression, const std::string&);

      static std::vector<std::pair<double,double>> get_pts(const expression&, const std::string&, double, double, double, double, int);

      static std::vector<std::tuple<double,double,double>> stat(const expression&, const std::string&, int);

      static std::vector<std::tuple<double,double,bool>> get_pts_tempered(const expression&, const std::string&, double, double, double,
        double, int, const expression&);

      // returns expression where x^(a/b) with a,b num, b odd is replaced by x/abs(x) * abs(x)^(a/b) -- needed to correctly evaluate x^(1/3) for negative x
      static expression graph_appropriate_expression(expression);

};

// polynomial division stuff, zero finding

class polynomial {
  public:
  // level is safeguard against too many gcd rec calls if == from expression doenst work properly
    static std::vector<expression> gcd(const std::vector<expression>&, const std::vector<expression>&, int=10);
    static std::pair<std::vector<expression>, std::vector<expression>> divmod(const std::vector<expression>&, const std::vector<expression>&);
    static std::vector<expression> zeros(std::vector<expression>, bool = false);
    static std::vector<expression> derivative(std::vector<expression>);
    static std::pair<std::vector<expression>,std::vector<std::tuple<expression,expression,int>>> 
         partial_fractions(std::vector<expression>, const std::vector<expression>&, const std::vector<expression>&);
    static std::vector<big_num_rational> rational(const std::vector<expression>&);
    static std::vector<big_num_rational> rational_zeros(const std::vector<big_num_rational>&);
    static std::vector<big_pos_int> divisors(const big_pos_int&);
    static expression to_expression(const std::vector<expression>&, const std::string&);
    static std::vector<expression> multiply(const std::vector<expression>&, const std::vector<expression>&);
    static std::vector<expression> sum(const std::vector<expression>&, const std::vector<expression>&);
    static std::vector<expression> minus(const std::vector<expression>&, const std::vector<expression>&);
    static void simplify(std::vector<expression>&);
};

class expression_const {
  public:
    std::set<char> _token_alphabet;
    std::vector<std::vector<std::pair<char, std::string>>> _binary_operators;
    std::vector<std::pair<char, std::string>> _unary_operators;
    std::map<expression::error_handling::error_code, std::string> _error_message;

    static expression_const& get_instance();

  private:
    expression_const();

};

#define unary_operators expression_const::get_instance()._unary_operators
#define binary_operators expression_const::get_instance()._binary_operators
#define token_alphabet expression_const::get_instance()._token_alphabet
#define error_message expression_const::get_instance()._error_message
