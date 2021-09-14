

#include <iostream>
#include <vector>
#include <utility>
#include <functional>
#include <complex>
#include <initializer_list>

#include "functions.hpp"

using cd = std::complex<double>;

       func::func() {}

       func::func(const std::string& f_name, int no_args, const std::function<cd(const std::vector<cd>&)>& f_eval,
             const std::function<std::pair<bool,algebraic_expression::fraction>(const std::vector<algebraic_expression::fraction>&)>& f_ev_frac,
             const std::function<expression(const std::vector<expression::eq_node*>&,int)>& f_der,
             const std::vector<std::string> bke,
             const std::function<bool(const algebraic_expression::fraction&, const algebraic_expression::fraction&)> symm_eq,
             const std::function<big_num_rational(const algebraic_expression::fraction&, const algebraic_expression::fraction&)> symm_coeff) :
                              name(f_name), args(no_args), eval(f_eval),
               eval_function(f_ev_frac), symmetry_equal(symm_eq), symmetry_coeff(symm_coeff), derivative(f_der), bookends(bke)  {}


    cd evaluator::evaluate(const expression& expr, const std::map<std::string, cd>& vars) {
        return evaluate_helper(expr.root, vars);
    }

    cd evaluator::evaluate_helper(const expression::eq_node* root, const std::map<std::string, cd>& vars) {   
         if (root==nullptr) { return cd(0,0); }
         if ((known_functions_dict.count(root->token_name)) &&
            ((known_functions_dict[root->token_name].args==-1) || (known_functions_dict[root->token_name].args==root->children.size()))) {
                    std::vector<cd> args;
                    for(auto c_n : root->children) {
                        args.push_back(evaluate_helper(c_n, vars));
                    }
                    return known_functions_dict[root->token_name].eval(args);
         }
         if (big_num::is_num(root->token_name)) {
                    return cd(std::stod(root->token_name),0);
         }
         if (vars.count(root->token_name)) {
             return vars.at(root->token_name);
         }
         check(false) << "unknown variable/function " << root->token_name << "\n";
         return cd(0,0);
    }


evaluator::evaluator() {

    // 0
    known_functions.emplace_back(id_func_name, 1, [](const std::vector<cd>& arg) { 
    return arg.front();
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(true, arg.front());
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1LL);
}, std::vector<std::string>{ "", "" }, 
   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});

  // 1
  known_functions.emplace_back(plus_func_name, -1, [](const std::vector<cd>& arg) { 
    cd temp(0);
    for(auto num : arg) {
        temp = temp + num;
    }
    return temp;
}, [](const std::vector<algebraic_expression::fraction>& arg) {
                algebraic_expression::fraction ret(0);
                for(const auto& t : arg) {
                    ret = ret + t;
                }
                return std::make_pair(true, ret);
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1LL);
}, std::vector<std::string>{},
   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
}
);

  // 2
  known_functions.emplace_back(minus_func_name, 2, [](const std::vector<cd>& arg) {
    return arg.front() - arg.back();
}, [](const std::vector<algebraic_expression::fraction>& arg) {
                return std::make_pair(true, arg.front() + algebraic_expression::fraction(-1) * arg.back());
}, [](const std::vector<expression::eq_node*>& args, int i){
  if (i==0) {
    return expression(1LL);
  } else {
    return expression(-1LL);
  }
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
}
);


  // 3
  known_functions.emplace_back(multiply_func_name, -1, [](const std::vector<cd>& arg) {
    cd temp(1);
    for(auto num : arg) {
        temp = temp * num;
    }
    return temp;
}, [](const std::vector<algebraic_expression::fraction>& arg) {
                algebraic_expression::fraction ret(1);
                for(const auto& t : arg) {
                    ret = ret * t;
                }
                return std::make_pair(true, ret);
}, [](const std::vector<expression::eq_node*>& args, int i){
  std::vector<expression> factors;
  for(int j = 0; j < args.size(); ++j) {
      if (j!=i) {
          factors.emplace_back(expression(args[j]));
      }
  }
  return expression::apply(multiply_func_name, factors);
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
}
);


  // 4
  known_functions.emplace_back(divide_func_name, 2, [](const std::vector<cd>& arg) {
    return arg.front()/arg.back();
}, [](const std::vector<algebraic_expression::fraction>& arg) {
                return std::make_pair(true, arg.front()/arg.back());
}, [](const std::vector<expression::eq_node*>& args, int i){
  if (i==0) {
    return expression(1)/expression(args[1]);
  } else {
    return -expression(args[0])/expression::apply(power_func_name, { expression(args[1]), expression(2LL) });
  }
}, std::vector<std::string>{ "\\frac{", "}{", "}" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});

  // 5
  known_functions.emplace_back(power_func_name, 2, [](const std::vector<cd>& arg) {
    return std::pow(arg.front(), arg.back());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  expression ret;
  if (i==0) {
      return expression(args[1]) * expression::apply(power_func_name, { expression(args[0]), expression(args[1])-expression(1) });
  } else {
      return expression::apply(ln_func_name, { expression(args[0]) }) * expression::apply(power_func_name, { expression(args[0]), expression(args[1]) });
  }
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 6
  known_functions.emplace_back(abs_func_name, 1, [](const std::vector<cd>& arg) {
    return std::abs(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(args[0])/expression::apply(abs_func_name, { expression(args[0]) });
}, std::vector<std::string>{  "\\left|", "\\right|" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 7
  // TODO: implement periodicity and special values with help of imaginary part when its done
  known_functions.emplace_back (exp_func_name, 1, [](const std::vector<cd>& arg) {
    return std::exp(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    check(!arg.front().denominator.empty()) << "zero denominator in exponent\n";

// somehow move this outside without making a gloabal or static var

map_bf<big_num_rational,algebraic_expression::fraction> special_vals_exp{ 
    { big_num_rational(1,3), 
       algebraic_expression::fraction(std::vector<algebraic_expression::term>{ algebraic_expression::term(1), 
         algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) * 
         algebraic_expression::term(3, algebraic_expression::fraction(big_num_rational(1,2))) }, 
       std::vector<algebraic_expression::term>{ algebraic_expression::term(2) })  },
    { big_num_rational(1,4), 
       algebraic_expression::fraction(std::vector<algebraic_expression::term>{ 
         algebraic_expression::term(2, algebraic_expression::fraction(big_num_rational(1,2))), 
         algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) * 
         algebraic_expression::term(2, algebraic_expression::fraction(big_num_rational(1,2))) }, 
       std::vector<algebraic_expression::term>{ algebraic_expression::term(2) })  },
    { big_num_rational(1,6), 
       algebraic_expression::fraction(std::vector<algebraic_expression::term>{ 
         algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) ,
         algebraic_expression::term(3, algebraic_expression::fraction(big_num_rational(1,2))) }, 
       std::vector<algebraic_expression::term>{ algebraic_expression::term(2) })  }
};

    if (arg.front().numerator.size()!=1) {
      return std::make_pair(true, algebraic_expression::fraction(std::vector<algebraic_expression::term>{ algebraic_expression::term(euler_number.name, arg.front()) }, 
                                                                 std::vector<algebraic_expression::term>{ algebraic_expression::term(1) }));
    }
    algebraic_expression::fraction pow(0);
    algebraic_expression::fraction new_factor(1);
    for(const auto& t_ : arg.front().numerator) {
        algebraic_expression::term num = t_ / (arg.front().denominator.front() * 
                                               algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) *
                                               algebraic_expression::term(pi_number.name, algebraic_expression::fraction(1)));

        if (num.vars.index.empty()) {
            num.coeff = big_num_rational(num.coeff.numerator % (num.coeff.denominator*2), num.coeff.denominator);
            if (num.coeff >= 1) {
                num.coeff = num.coeff - 1;
                new_factor = new_factor * algebraic_expression::fraction(-1);
            }
            if (num.coeff >= big_num_rational(1,2)) {
                num.coeff = num.coeff - big_num_rational(1,2);
                new_factor = new_factor * algebraic_expression::fraction(std::vector<algebraic_expression::term>{ 
                                       algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) },
                          std::vector<algebraic_expression::term>{ algebraic_expression::term(1) });
            }
            if (special_vals_exp.exists(num.coeff)) {
                new_factor = new_factor * special_vals_exp[num.coeff];
                num.coeff = 0;
            }
        }
        pow = pow + algebraic_expression::fraction(std::vector<algebraic_expression::term>{ 
                                            num * algebraic_expression::term(imaginary_unit.name, algebraic_expression::fraction(1)) *
                                                  algebraic_expression::term(pi_number.name, algebraic_expression::fraction(1)) },
                                            std::vector<algebraic_expression::term>{ algebraic_expression::term(1) }   );

    }
    return std::make_pair(true, new_factor*algebraic_expression::fraction(std::vector<algebraic_expression::term>{ algebraic_expression::term(euler_number.name, pow) },
                                                     std::vector<algebraic_expression::term>{ algebraic_expression::term(1) }));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression::apply(exp_func_name, { expression(args[0]) });
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 8
  known_functions.emplace_back(sqrt_unary_func_name, 1, [](const std::vector<cd>& arg) {
    return std::sqrt(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1LL)/(expression(2)*expression::apply(sqrt_unary_func.name, {expression(args[0])}));
}, std::vector<std::string>{ "\\sqrt{", "}" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});

  // 9
  known_functions.emplace_back(sqrt_func_name, 2, [](const std::vector<cd>& arg) {
    return std::pow(arg.back(), cd(1,0)/arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  if (i==0) {
    return -expression::apply(ln_func_name, {expression(args[1])}) * expression::apply(sqrt_func_name, { expression(args[0]), expression(args[1]) })
       /expression::apply(power_func_name, {expression(args[0]), expression(2)});
  } else {
    return expression(1LL) * expression::apply(sqrt_func_name, { expression(args[0]), expression(args[1]) })/(expression(args[0]) * expression(args[1]));
  }
}, std::vector<std::string>{ "\\sqrt[", "]{", "}" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 10
  known_functions.emplace_back(minus_unary_func_name, 1, [](const std::vector<cd>& arg) {
    return -arg.front();
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(-1);
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 11
  known_functions.emplace_back(ln_func_name, 1, [](const std::vector<cd>& arg){
    return std::log(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1)/expression(args[0]);
}, std::vector<std::string>{},   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 12
  known_functions.emplace_back(sin_func_name, 1, [](const std::vector<cd>& arg) {
    return std::sin(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression::apply(cos_func_name, {expression(args[0])});
}, std::vector<std::string>{  }, 
   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    if (a==b) {
        return big_num_rational(1);
    } else {
        return big_num_rational(-1);
    }
});


  // 13
  known_functions.emplace_back(cos_func_name, 1, [](const std::vector<cd>& arg) {
    return std::cos(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return -expression::apply(sin_func_name, { expression(args[0]) });
}, std::vector<std::string>{  },
   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return big_num_rational(1);
});


  // 14
  known_functions.emplace_back(euler_number_name, 0, [](const std::vector<cd>& arg) {
    return cd(2.7182818284590452353602874713527,0);
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(0LL);
}, std::vector<std::string>{ "e" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 15
  known_functions.emplace_back(pi_number_name, 0, [](const std::vector<cd>& arg) {
    return cd(3.14159265359,0);
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(0LL);
}, std::vector<std::string>{ "\\pi" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 16
  known_functions.emplace_back(imaginary_unit_name, 0, [](const std::vector<cd>& arg) {
    return cd(0,1);
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(0LL);
}, std::vector<std::string>{ "i" },   [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return (a==b);
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
        return big_num_rational(1);
});


  // 17
  known_functions.emplace_back(tan_func_name, 1, [](const std::vector<cd>& arg) {
    return std::tan(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1)/expression::apply(power_func_name, { expression::apply(cos_func_name, {expression(args[0])}), expression(2)});
}, std::vector<std::string>{}, 
[](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    if (a==b) {
        return big_num_rational(1);
    } else {
        return big_num_rational(-1);
    }
});

  // 18
  known_functions.emplace_back(arcsin_func_name, 1, [](const std::vector<cd>& arg) {
    return std::asin(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1)/expression::apply(sqrt_unary_func_name, { expression(1LL) - expression::apply(power_func_name, {expression(args[0]), expression(2)})});
}, std::vector<std::string>{}, 
[](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    if (a==b) {
        return big_num_rational(1);
    } else {
        return big_num_rational(-1);
    }
});

  // 19
  known_functions.emplace_back(arccos_func_name, 1, [](const std::vector<cd>& arg) {
    return std::acos(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return -expression(1)/expression::apply(sqrt_unary_func_name, { expression(1LL) - expression::apply(power_func_name, {expression(args[0]), expression(2)})});
}, std::vector<std::string>{}, 
[](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
      return big_num_rational(1);
});

  // 20
  known_functions.emplace_back(arctan_func_name, 1, [](const std::vector<cd>& arg) {
    return std::atan(arg.front());
}, [](const std::vector<algebraic_expression::fraction>& arg) {
    return std::make_pair(false, algebraic_expression::fraction(0));
}, [](const std::vector<expression::eq_node*>& args, int i){
  return expression(1)/(expression(1) + expression::apply(power_func_name, {expression(args[0]), expression(2)}));
}, std::vector<std::string>{}, 
[](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    return ((a==b) || (a+b==algebraic_expression::fraction(0)));
}, [](const algebraic_expression::fraction& a, const algebraic_expression::fraction& b) {
    if (a==b) {
        return big_num_rational(1);
    } else {
        return big_num_rational(-1);
    }
});



      for(const auto& f : known_functions) {
          known_functions_dict[f.name] = f;
      }
}

    evaluator& evaluator::get_instance() {
      static evaluator inst;
      return inst;
    }


eratostenes::eratostenes(int n) : N(n), lp(N+1, 0) {
   for (int i=2; i<=N; ++i) {
     if (lp[i] == 0) {
     lp[i] = i;
     pr.push_back(i);
   }
   for (int j=0; j<(int)pr.size() && pr[j]<=lp[i] && i*pr[j]<=N; ++j)
     lp[i * pr[j]] = pr[j];
   }
}

eratostenes& eratostenes::get_instance() {
  static eratostenes inst(10000000);
  return inst;
};
