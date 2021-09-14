
// TODO: suggestions for speeding up common denominator: in big_num use ll for small numbers, and strings only for big ones;
//       try map_bf map, try array for small arrays and vector for large ones (everywhere and in map_bf)

#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <stack>
#include <set>
#include <functional>

#include "bignum.hpp"
#include "misc.hpp"
#include "functions.hpp"


int main(int argc, char *argv[])   {

     std::string s;
     getline(std::cin, s);
     expression expr(s);
     if (expr.parsing_error.error != expression::error_handling::ok) {     
       std::cout << "Error: " << error_message.at(expr.parsing_error.error) << std::endl;
       std::cout << s << std::endl;
       std::cout << std::string(expr.parsing_error.position_of_error, ' ') + '^' << std::endl;
       return 0;
     }

//     std::cout << "eval " << default_eval.evaluate(expr, {}) << std::endl;
     std::cout << "raw "; expr.print_raw();
     std::cout << "latex " << expr.to_latex() << std::endl;
     std::cout << "simplified "; expression(expr).simplify_rational().print_raw();
     expression qq = graph::graph_appropriate_expression(expr);
     std::cout << "graph appropriate expression "; qq.print_raw();

     algebraic_expression rr(expr, "x");
     std::cout << "num ";
     std::vector<expression> coeff;
     for(auto x : rr.num_coeff) {
         coeff.push_back(x.to_expression(rr.dependent_vars, rr.func_arg));
         coeff.back().print();
     }
     std::cout << "denom ";
     std::vector<expression> coeff2;
     for(auto x : rr.denom_coeff) {
         coeff2.push_back(x.to_expression(rr.dependent_vars, rr.func_arg));
         coeff2.back().print();
     }
//     auto qq = polynomial::gcd(coeff, coeff2);
//     for(auto x : qq) {
//       x.print_raw();
//     }
/*     auto q = polynomial::divmod(coeff, coeff2);
     std::cout << "div" << std::endl;
     for(auto x : q.first) {
       x.simplify_rational().print_raw();
     }
     std::cout << "mod" << std::endl;
     for(auto x : q.second) {
       x.simplify_rational().print_raw();
     } */

     std::cout << "derivative " << std::endl;
     expression(expr).derivative("x").print_raw();

     expr.simplify_rational("x");
     std::cout << "canceled rational function "; expr.print();

     std::cout << "solutions" << std::endl;
     if (!coeff2.empty()) {
       auto z = polynomial::zeros(coeff2);
       for(auto x : z) {
         x.print();
       }

       std::cout << "part frac" << std::endl;

       auto pf = polynomial::partial_fractions(coeff, coeff2, z);
       for(auto x : pf.second) {
         std::cout << std::get<2>(x) << " ";
         std::get<0>(x).simplify_rational().print_raw();
         std::get<1>(x).simplify_rational().print_raw();
       }
     }

     return 0; 

}
