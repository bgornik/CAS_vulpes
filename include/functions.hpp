

#include <iostream>
#include <vector>
#include <utility>
#include <functional>
#include <complex>
#include <initializer_list>

#include "expression.hpp"

using cd = std::complex<double>;

// TODO: introduce func application operator to class expression and rewrite all derivative definitions below accordinglx


class func {
      public:
        std::string name;

        int args;                 // number of arguments; if -1, any number of arguments allowed

        std::function<cd(const std::vector<cd>&)> eval;

        std::function<std::pair<bool,algebraic_expression::fraction>(const std::vector<algebraic_expression::fraction>&)> eval_function;

          // the following function is used for grouping arguments of a function that correspond to results expressed by rational multiple;
          // need not be implemented if eval_functions always returns (true,..)
        std::function<bool(const algebraic_expression::fraction&,const algebraic_expression::fraction&)> symmetry_equal;

          // returns rational coeff from above comment; should only be called on args where above func returns true
        std::function<big_num_rational(const algebraic_expression::fraction&,const algebraic_expression::fraction&)> symmetry_coeff;

        std::function<expression(const std::vector<expression::eq_node*>&,int)> derivative;

        std::vector<std::string> bookends;

        func();

        func(const std::string& f_name, int no_args, const std::function<cd(const std::vector<cd>&)>& f_eval,
             const std::function<std::pair<bool,algebraic_expression::fraction>(const std::vector<algebraic_expression::fraction>&)>& f_ev_frac,
             const std::function<expression(const std::vector<expression::eq_node*>&,int)>& f_der,
             const std::vector<std::string> bke,
             const std::function<bool(const algebraic_expression::fraction&, const algebraic_expression::fraction&)> symm_eq,
             const std::function<big_num_rational(const algebraic_expression::fraction&, const algebraic_expression::fraction&)> symm_coeff);

};

class evaluator {
  public:
    std::vector<func> known_functions;
    std::map<std::string, func> known_functions_dict;

    cd evaluate(const expression&, const std::map<std::string, cd>&);

    cd evaluate_helper(const expression::eq_node*, const std::map<std::string, cd>&);

    static evaluator& get_instance();

    private:
      evaluator();

};


#define id_func          evaluator::get_instance().known_functions[0]
#define id_func_name      "__id_"
#define plus_func        evaluator::get_instance().known_functions[1]
#define plus_func_name   "__plus_"
#define minus_func       evaluator::get_instance().known_functions[2]
#define minus_func_name  "__minus_"
#define multiply_func    evaluator::get_instance().known_functions[3]
#define multiply_func_name "__multiply_"
#define divide_func      evaluator::get_instance().known_functions[4]
#define divide_func_name "__divide_"
#define power_func       evaluator::get_instance().known_functions[5]
#define power_func_name  "__power_"
#define abs_func         evaluator::get_instance().known_functions[6]
#define abs_func_name    "abs"
#define exp_func         evaluator::get_instance().known_functions[7]
#define exp_func_name    "exp"
#define sqrt_unary_func  evaluator::get_instance().known_functions[8]
#define sqrt_unary_func_name  "sqrt"
#define sqrt_func        evaluator::get_instance().known_functions[9]
#define sqrt_func_name   "__sqrt_"
#define minus_unary_func evaluator::get_instance().known_functions[10]
#define minus_unary_func_name "__negate_"
#define ln_func          evaluator::get_instance().known_functions[11]
#define ln_func_name     "ln"
#define sin_func         evaluator::get_instance().known_functions[12]
#define sin_func_name    "sin"
#define cos_func         evaluator::get_instance().known_functions[13]
#define cos_func_name    "cos"
#define euler_number     evaluator::get_instance().known_functions[14]
#define euler_number_name "e"
#define pi_number        evaluator::get_instance().known_functions[15]
#define pi_number_name   "pi"
#define imaginary_unit   evaluator::get_instance().known_functions[16]
#define imaginary_unit_name "__i_"
#define tan_func         evaluator::get_instance().known_functions[17]
#define tan_func_name    "tg"
#define arcsin_func         evaluator::get_instance().known_functions[18]
#define arcsin_func_name    "arcsin"
#define arccos_func         evaluator::get_instance().known_functions[19]
#define arccos_func_name    "arccos"
#define arctan_func         evaluator::get_instance().known_functions[20]
#define arctan_func_name    "arctg"

#define default_eval     evaluator::get_instance()

class eratostenes {
    public:
      int N;
      std::vector<int> lp, pr;  // lp[i] = smallest prime that divides i; pr -- array of primes

      static eratostenes& get_instance();

    private:
      eratostenes(int);
};

#define erat eratostenes::get_instance()
