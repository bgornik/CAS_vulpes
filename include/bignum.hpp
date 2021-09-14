#pragma once

#include <iostream>
#include <string>
#include "misc.hpp"


class big_pos_int {

    public:
        // status after parsing in the constructor; 0=OK, 1=given string includes characters which are not 0-9; 2=given number is negative
      int err_code;

      big_pos_int();
      big_pos_int(long long x);
      big_pos_int(const std::string& s); 

      big_pos_int operator+(const big_pos_int& y) const;
      big_pos_int operator-(const big_pos_int& y) const;
      big_pos_int operator*(big_pos_int y) const;
      big_pos_int operator/(const big_pos_int& y) const;
      big_pos_int operator%(const big_pos_int& y) const;
      bool operator==(const big_pos_int& y) const;
      bool operator!=(const big_pos_int& y) const;
      bool operator<(const big_pos_int& y) const;
      bool operator>(const big_pos_int& y) const;
      bool operator<=(const big_pos_int& y) const;
      bool operator>=(const big_pos_int& y) const;
      big_pos_int operator^(big_pos_int y) const;
      std::pair<bool,big_pos_int> take_root(const big_pos_int&) const;

        // are all characters '0'-'9' ?
      static bool is_num(const std::string& s);
      std::string to_string() const;
      std::string num;
      static big_pos_int gcd(const big_pos_int&, const big_pos_int&);

    private:

      static big_pos_int mult_helper(const std::string& s, int dig);
      static char div_helper(const std::string& s, const std::string& d);
      static std::pair<big_pos_int, big_pos_int> divmod(const std::string& s, const std::string& d);
};

std::ostream& operator<<(std::ostream& out, const big_pos_int& x);

class big_int : public big_pos_int {
    public:
        // int err_code: status after parsing in the constructor; 0=OK, 1=given string includes characters which are not 0-9 with possibly minus sign in first place; 

      big_int();
      big_int(long long x);
      big_int(const std::string& s); 
      big_int(const big_pos_int& x);
      big_int(const big_pos_int& x, bool sign);

      big_int operator+(const big_int& y) const;
      big_int operator-(const big_int& y) const;
      big_int operator*(const big_int& y) const;
      big_int operator/(const big_int& y) const;
      big_pos_int operator%(const big_pos_int& y) const;
      big_int operator-() const;
      bool operator==(const big_int& y) const;
      bool operator!=(const big_int& y) const;
      bool operator<(const big_int& y) const;
      bool operator>(const big_int& y) const;
      bool operator<=(const big_int& y) const;
      bool operator>=(const big_int& y) const;

        // are all characters '0'-'9' ? (with minus allowed at beginning)
      static bool is_num(const std::string& s);
      static big_int pow10(int digs);
      std::string to_string() const;

      bool positive;
};

std::ostream& operator<<(std::ostream& out, const big_int& x);
big_pos_int abs(const big_int& y);

class big_num : public big_int {
      // this class represents number big_int/10^power_of_ten

    public:
        // int err_code: status after parsing in the constructor; 0=OK
        // 1=given string includes characters which are not 0-9 with possibly minus sign in first place and one dot

      big_num();
      big_num(long long x);
      big_num(const std::string& s);
      big_num(const big_int& x);
      big_num(const big_int& x, int pow10);

      big_num operator+(const big_num& y) const;
      big_num operator-(const big_num& y) const;
      big_num operator*(const big_num& y) const;
      big_num operator/(const big_num& y) const;
      big_num operator-() const;
       
      bool operator==(const big_num& y) const;
      bool operator!=(const big_num& y) const;
      bool operator<(const big_num& y) const;
      bool operator>(const big_num& y) const;
      bool operator<=(const big_num& y) const;
      bool operator>=(const big_num& y) const;

        // are all characters '0'-'9' ? (with minus allowed at beginning + one dot)
      static bool is_num(const std::string& s);
      static bool is_int(const big_num& x);
      std::string to_string() const;
      int power_of_ten;
      void simplify();
      static big_num gcd(const big_num& x, const big_num& y);

};

std::ostream& operator<<(std::ostream& out, const big_num& x);
big_num abs(const big_num& x);

class big_num_rational {
      // this class represents a rational number bigint/bigposint

    public:
      int err_code; // status after parsing in the constructor; 0=OK, 1=not OK, 2=denominator==0

      big_int numerator;
      big_pos_int denominator;

      big_num_rational();
      big_num_rational(long long x);
      big_num_rational(big_int num, big_int den);
      big_num_rational(const big_num& x);

      big_num_rational operator+(const big_num_rational&) const;
      big_num_rational operator-(const big_num_rational&) const;
      big_num_rational operator*(const big_num_rational&) const;
      big_num_rational operator/(const big_num_rational&) const;
      big_num_rational operator-() const;
       
      bool operator==(const big_num_rational& y) const;
      bool operator!=(const big_num_rational& y) const;
      bool operator<(const big_num_rational& y) const;
      bool operator>(const big_num_rational& y) const;
      bool operator<=(const big_num_rational& y) const;
      bool operator>=(const big_num_rational& y) const;

        // are all characters '0'-'9' ? (with minus allowed at beginning + one dot)
      static bool is_int(const big_num_rational& x);
      void simplify();
      static big_num_rational gcd(const big_num_rational& x, const big_num_rational& y);

};

std::ostream& operator<<(std::ostream& out, const big_num_rational& x);
big_num_rational abs(const big_num_rational& x);

