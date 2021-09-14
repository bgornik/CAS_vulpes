
#include <iostream>
#include <string>
#include <algorithm>

#include "bignum.hpp"


big_pos_int::big_pos_int() : num("0"), err_code(0) {}

big_pos_int::big_pos_int(long long x) : err_code(0) {
          if (x<0) {
              err_code = 2;
              return;
          }
          while (x>0) {
              num += '0' + (x%10);
              x /= 10;
          }
          reverse(num.begin(), num.end());
          if (num.empty()) {
              num = "0";
          }
}


big_pos_int::big_pos_int(const std::string& s) : err_code(0) {
        if (!is_num(s)) {
            err_code = 1;
            return;
        }
        int i = 0;
        while ((i < s.size()) && (s[i]=='0')) { ++i; }
        num = s.substr(i);
        if (num.empty()) {
            num = "0";
        }
}

std::pair<big_pos_int, big_pos_int> big_pos_int::divmod(const std::string& s, const std::string& d) {
    big_pos_int retd, retm;
    int i = 0; retm.num = "";
    while (i < s.size()) {
        retm.num += s[i];
        retd.num += div_helper(retm.num, d);
        retm = retm - big_pos_int(retd.num.back() - '0') * d;
        ++i;
    }
    i = 0;
    while ((i < retd.num.size()) && (retd.num[i] == '0')) { ++i; }
    if (i==retd.num.size()) { --i;  }
    retd.num = retd.num.substr(i);
    return std::make_pair(retd, retm);
}


char big_pos_int::div_helper(const std::string& s, const std::string& d) {
    int start = 0, end = 10;
    while (end-start>1) {
        int mid = (start+end)/2;
        if (big_pos_int(d)*mid > s) {  end = mid;  } else { start = mid;   }
    }
    return start + '0';
}


big_pos_int big_pos_int::operator+(const big_pos_int& y) const {
          int len_ret = std::max(y.num.size(), num.size()) + 1;
          big_pos_int ret;
          ret.num = std::string(std::max(0, len_ret - static_cast<int>(num.size())), '0') + num;

          bool carry = false;
          for(int i = ret.num.size()-1; i >= 0; --i) {
              int digit1 = ret.num[i] - '0';
              int digit2 = carry;
              int i_y = i - static_cast<int>(ret.num.size()) + y.num.size();
              if (i_y >= 0) {
                digit2 += y.num[i_y] - '0';
              }
              carry = (digit1+digit2) >= 10;
              ret.num[i] = '0' + (digit1+digit2) % 10;
          }
          if (ret.num[0]=='0') {
              ret.num = ret.num.substr(1);
          }
          return ret;
}

// is guaranteed to give meaningful result only if this>=y

big_pos_int big_pos_int::operator-(const big_pos_int& y) const {
    big_pos_int ret(*this);
    std::string y0 = std::string(ret.num.size() - y.num.size(), '0') + y.num;

    bool carry = false;
    for(int i = ret.num.size()-1; i >= 0; --i) {
        int d = static_cast<int>(ret.num[i]) - static_cast<int>(y0[i]) - carry;
        carry = (d<0);
        d = (d+10) % 10;
        ret.num[i] = d + '0';
    }
    int i = 0;
    while ((i < ret.num.size()) && (ret.num[i] == '0')) { ++i; }
    if (i==ret.num.size()) { --i;  }
    ret.num = ret.num.substr(i);
    return ret;
}

big_pos_int big_pos_int::operator*(big_pos_int y) const {
          big_pos_int ret = 0, pow10 = 1;
          for(int i = y.num.size()-1; i >= 0; --i) {
              big_pos_int temp = mult_helper(num, y.num[i]-'0');
              if (temp!=0) {
                temp.num += std::string(y.num.size()-1-i, '0');
              }
              ret = ret + temp;
          }
          return ret;
}

bool big_pos_int::operator<(const big_pos_int& y) const {
    if (this->num.size() != y.num.size()) {
        return (this->num.size() < y.num.size());
    }
    int i = 0;
    while (i<this->num.size()) {
        if (this->num[i]!=y.num[i]) {
            break;
        }
        ++i;
    }
    return ((i<this->num.size()) && (this->num[i]<y.num[i]));
}

bool big_pos_int::operator>(const big_pos_int& y) const {
    return big_pos_int(y) < *this;
}

bool big_pos_int::operator<=(const big_pos_int& y) const {
    return ((*this < y) || (*this==y));
}

bool big_pos_int::operator>=(const big_pos_int& y) const {
    return ((*this > y) || (*this==y));
}


bool big_pos_int::operator==(const big_pos_int& y) const {
    return num==y.num;
}

bool big_pos_int::operator!=(const big_pos_int& y) const {
    return !(*this==y);
}

big_pos_int big_pos_int::operator/(const big_pos_int& y) const {
    check(y!=0) << "division by 0 in bignum library\n";
    return divmod(num, y.num).first;
}

big_pos_int big_pos_int::operator%(const big_pos_int& y) const {
    check(y!=0) << "division by 0 in bignum library\n";
    return divmod(num, y.num).second;
}

bool big_pos_int::is_num(const std::string& s) {
          bool is_num = true;
          for(auto c : s) {
              is_num &= ((c >= '0') && (c <= '9'));
          }
          return is_num;
}

big_pos_int big_pos_int::mult_helper(const std::string& s, int dig) {
    if ((dig==0) || (s=="0")) {
        return 0;
    }
    std::string ret;
    int carry = 0;
    for(int i = s.size()-1; i >= 0; --i) {
        int d = (s[i] - '0')*dig + carry;
        ret += ('0' + d%10);
        carry = d/10;
    }
    if (carry>0) {
        ret += '0' + carry;
    }
    reverse(ret.begin(), ret.end());
    return big_pos_int(ret);
}

std::string big_pos_int::to_string() const {
    return num;
}

big_pos_int big_pos_int::gcd(const big_pos_int& a, const big_pos_int& b) {
   if (b==0) { return a; } else { return gcd(b, a % b); }
}


std::ostream& operator<<(std::ostream& out, const big_pos_int& x) {
    return (out << x.to_string());
}

big_pos_int big_pos_int::operator^(big_pos_int y) const {
    big_pos_int w(*this), res = 1;
    while (y>0) {
        if (y%2!=0) {
            res = (res * w);
        }
        y = y/2;
        w = (w * w);
    }
    return res;
}

std::pair<bool,big_pos_int> big_pos_int::take_root(const big_pos_int& b) const {
    big_pos_int L = 1;
    big_pos_int R = *this+1;
    while (L < R) {
        big_pos_int m = (L + R) / 2;
        big_pos_int val = m^b;
        if (val < *this) L = m + 1;
          else if (val > *this) R = m;
          else return std::make_pair(true, m);
    }
    return std::make_pair(false, 0);
}



big_int::big_int() : big_pos_int(), positive(true) {}

big_int::big_int(long long x) : big_pos_int((x>=0 ? x : (-x))), positive(x >= 0) {}

big_int::big_int(const std::string& s) : big_pos_int((s[0]=='-') ? s.substr(1) : s), positive(s[0]!='-') {}

big_int::big_int(const big_pos_int& x) : big_pos_int(x), positive(true) {}

big_int::big_int(const big_pos_int& x, bool sign) : big_pos_int(x), positive(sign) {}

big_pos_int abs(const big_int& y) {  
    return static_cast<big_pos_int>(y);
}

big_int big_int::operator-() const {
    return big_int(static_cast<big_pos_int>(*this), !positive);
}

big_int big_int::operator+(const big_int& y) const {
    if ((positive && y.positive) || (!positive && !y.positive)) {
        return big_int(static_cast<big_pos_int>(*this) + static_cast<big_pos_int>(y), positive);
    }
    big_pos_int temp_val;
    bool sign;
    if (abs(*this)>abs(y)) {  
        temp_val = abs(*this) - abs(y);
        sign = positive;
    } else {
        temp_val = abs(y) - abs(*this);
        sign = !positive;
    }
    return big_int(temp_val, sign);
}

big_int big_int::operator-(const big_int& y) const {
    return *this+(-y);
}

big_int big_int::operator*(const big_int& y) const {
    return big_int(static_cast<big_pos_int>(*this) * static_cast<big_pos_int>(y), positive==y.positive);
}

big_int big_int::operator/(const big_int& y) const {
    big_pos_int num(static_cast<big_pos_int>(*this)), den(static_cast<big_pos_int>(y));
    return big_int(num/den, positive==y.positive);
}

bool big_int::operator==(const big_int& y) const {
    // case of zero hmust be handled separately since +0 and -0 is the same

    if ((static_cast<big_pos_int>(*this)==0) || (static_cast<big_pos_int>(y)==0)) {
        return ((static_cast<big_pos_int>(*this)==0) && (static_cast<big_pos_int>(y)==0));
    }
    return ((static_cast<big_pos_int>(*this)==static_cast<big_pos_int>(y)) && (positive==y.positive));
}

bool big_int::operator!=(const big_int& y) const {
    return !(*this==y);
}

bool big_int::operator<(const big_int& y) const {
    big_int temp_diff = y - *this;
    return (temp_diff.positive && (static_cast<big_pos_int>(temp_diff)>0));
}

bool big_int::operator>(const big_int& y) const {
    return (y<*this);
}

bool big_int::operator<=(const big_int& y) const {
    return ((*this<y) || (*this==y));
}

bool big_int::operator>=(const big_int& y) const {
    return ((*this>y) || (*this==y));
}

bool big_int::is_num(const std::string& s) {
    int i = (s[0]=='-' ? 1 : 0);
    bool is_ok = true;
    for(; i < s.size(); ++i) {
        is_ok &= ((s[i]>='0') && (s[i]<='9'));
    }
    return is_ok;
}

std::string big_int::to_string() const {
    std::string ret;
    if (*this==0) {
        ret = "0";
    } else {
        if (positive>0) {
          ret = static_cast<big_pos_int>(*this).to_string();
        } else {
          ret = "-" + static_cast<big_pos_int>(*this).to_string();
        }
    }
    return ret;
}

std::ostream& operator<<(std::ostream& out, const big_int& x) {
    return (out << x.to_string());
}

big_int big_int::pow10(int digs) {
    return big_int("1"+std::string(digs, '0'));
}


big_num::big_num() : big_int(), power_of_ten(0) {}

big_num::big_num(long long x) : big_int(x), power_of_ten(0) {}

big_num::big_num(const std::string& s) : big_int(((s.find('.') == std::string::npos) ? s : s.substr(0, s.find('.'))+s.substr(s.find('.')+1))),
                                         power_of_ten((s.find('.') == std::string::npos) ? 0 : s.size()-1-s.find('.'))    {
                                         simplify();
}

big_num::big_num(const big_int& x) : big_int(x), power_of_ten(0) {
    simplify();
}

big_num::big_num(const big_int& x, int pow10) : big_int(x), power_of_ten(pow10) {
    simplify();
}

void big_num::simplify() {
    if (static_cast<big_int>(*this)==0) {
        power_of_ten = 0;
        return;
    }
    while ((num.back()=='0') && (power_of_ten>0)) {
        num.pop_back();
        --power_of_ten;
    }
}

big_num big_num::operator+(const big_num& y) const {
    int pow10_common = std::max(power_of_ten, y.power_of_ten);
    big_num ret = big_num(static_cast<big_int>(*this) * big_int::pow10(pow10_common-power_of_ten) + 
                          static_cast<big_int>(y) * big_int::pow10(pow10_common-y.power_of_ten), pow10_common);
    ret.simplify();
    return ret;
}

big_num big_num::operator-(const big_num& y) const {
    int pow10_common = std::max(power_of_ten, y.power_of_ten);
    big_num ret = big_num(static_cast<big_int>(*this) * big_int::pow10(pow10_common-power_of_ten) - 
                          static_cast<big_int>(y) * big_int::pow10(pow10_common-y.power_of_ten), pow10_common);
    ret.simplify();
    return ret;
}

big_num big_num::operator*(const big_num& y) const {
    big_num ret = big_num(static_cast<big_int>(*this) * static_cast<big_int>(y), power_of_ten+y.power_of_ten);
    ret.simplify();
    return ret;
}

// warning: only works correctly if result an integer !!!
big_num big_num::operator/(const big_num& y) const {
    big_pos_int numerator(num), denominator(y.num);
    numerator.num = numerator.num + std::string(y.power_of_ten, '0');
    denominator.num = denominator.num + std::string(power_of_ten, '0');
    big_num ret(numerator/denominator);
    ret.positive = (positive == y.positive);
    ret.simplify();
    return ret;
}

bool big_num::is_num(const std::string& s) {
    int i = (s[0]=='-' ? 1 : 0);
    bool is_ok = true;
    int ct_dots = 0;
    for(; i < s.size(); ++i) {
        is_ok &= (((s[i]>='0') && (s[i]<='9')) || (s[i]=='.'));
        ct_dots += (s[i]=='.');
    }
    return (is_ok && (ct_dots <= 1));
}

std::ostream& operator<<(std::ostream& out, const big_num& x) {
   return (out << x.to_string());
}

big_num big_num::operator-() const {
   return big_num(-static_cast<big_int>(*this), power_of_ten);
}

bool big_num::operator==(const big_num& y) const {
    return (static_cast<big_int>(*this) * big_int::pow10(y.power_of_ten)) == (static_cast<big_int>(y) * big_int::pow10(power_of_ten));
}

bool big_num::operator!=(const big_num& y) const {
    return !(*this==y);
}

bool big_num::operator<(const big_num& y) const {
    return (static_cast<big_int>(*this) * big_int::pow10(y.power_of_ten)) < (static_cast<big_int>(y) * big_int::pow10(power_of_ten));
}

bool big_num::operator>(const big_num& y) const {
    return (y < *this);
}

bool big_num::operator<=(const big_num& y) const {
    return ((*this < y) || (*this==y));
}

bool big_num::operator>=(const big_num& y) const {
    return (y <= *this);
}

std::string big_num::to_string() const {
   std::string ret;
   if (power_of_ten==0) {
       return static_cast<big_int>(*this).to_string();
   }
   int num_str_len_wo_minus = num.size() - (num[0]=='-');
   std::string num_temp = std::string(std::max(power_of_ten+1 - num_str_len_wo_minus, 0), '0') + num;
   num_temp = num_temp.substr(0, num_temp.size() - power_of_ten) + "." + num_temp.substr(num_temp.size() - power_of_ten);
   if (positive) {
       ret = num_temp;
   } else {
       ret = "-" + num_temp;
   }
   return ret;
}

bool big_num::is_int(const big_num& x) {
    return (static_cast<big_pos_int>(x) % big_int::pow10(x.power_of_ten) == 0);
}

big_num abs(const big_num& x) {
    big_num ret(x);
    ret.positive = true;
    return ret;
}

big_num big_num::gcd(const big_num& x, const big_num& y) {
    big_pos_int a = static_cast<big_pos_int>(x), b = static_cast<big_pos_int>(y);
    if (x.power_of_ten>y.power_of_ten) {
        b.num = b.num + std::string(x.power_of_ten-y.power_of_ten, '0');
    } else {
        a.num = a.num + std::string(y.power_of_ten-x.power_of_ten, '0');
    }
    big_pos_int result = big_pos_int::gcd(a,b);
    big_num ret(result);
    ret.power_of_ten = std::max(x.power_of_ten, y.power_of_ten);
    ret.positive = true;
    ret.simplify();
    return ret;
}


big_num_rational::big_num_rational() : numerator(0), denominator(1), err_code(0) {}

big_num_rational::big_num_rational(big_int num, big_int den) : err_code(num.err_code || den.err_code)  {
          if (den<0) { num = -num; den = -den; }
          if (den==0) { err_code = 2; return; }
          numerator = num;
          denominator = den;
          simplify();
      }

big_num_rational::big_num_rational(const big_num& x) : 
      numerator(static_cast<big_int>(x)), denominator(big_int::pow10(x.power_of_ten)), err_code(x.err_code) {
           simplify();
      }

big_num_rational::big_num_rational(long long x) : numerator(x), denominator(1), err_code(0) {}


big_num_rational big_num_rational::operator+(const big_num_rational& y) const {
  return big_num_rational(numerator*y.denominator + y.numerator*denominator, denominator*y.denominator);
}

big_num_rational big_num_rational::operator-(const big_num_rational& y) const {
  return big_num_rational(numerator*y.denominator - y.numerator*denominator, denominator*y.denominator);
}

big_num_rational big_num_rational::operator*(const big_num_rational& y) const {
  return big_num_rational(numerator*y.numerator, denominator*y.denominator);
}

big_num_rational big_num_rational::operator/(const big_num_rational& y) const {
  if (y.numerator<0) { return big_num_rational(-numerator*y.denominator, denominator*(-y.numerator));  }
  return big_num_rational(numerator*y.denominator, denominator*y.numerator);
}

big_num_rational big_num_rational::operator-() const {
  return big_num_rational(-numerator, denominator);
}

bool big_num_rational::operator==(const big_num_rational& y) const {
    return numerator*y.denominator == y.numerator*denominator;
}

bool big_num_rational::operator!=(const big_num_rational& y) const {
    return !(*this==y);
}

bool big_num_rational::operator<(const big_num_rational& y) const {
   auto temp1 = -(numerator*y.denominator - y.numerator*denominator);
   auto temp2 = denominator*y.denominator;
   return (((temp1>0) && (temp2>0)) || ((temp1<0) && (temp2<0)));
}

bool big_num_rational::operator>(const big_num_rational& y) const {
    return (y < *this);
}

bool big_num_rational::operator<=(const big_num_rational& y) const {
    return ((*this<y) || (*this==y));
}

bool big_num_rational::operator>=(const big_num_rational& y) const {
    return ((y<*this) || (*this==y));
}

bool big_num_rational::is_int(const big_num_rational& x) {
    return x.denominator==1;
}

void big_num_rational::simplify() {
    big_pos_int ggcd = big_pos_int::gcd(static_cast<big_pos_int>(numerator), denominator);
    numerator = numerator/big_int(ggcd);
    denominator = denominator/ggcd;
}

std::ostream& operator<<(std::ostream& out, const big_num_rational& x) {
    return (out << x.numerator << "/" << x.denominator);
}

big_num_rational abs(const big_num_rational& x) {
    big_num_rational temp(x);
    temp.numerator = abs(temp.numerator);
    return temp;
}

big_num_rational big_num_rational::gcd(const big_num_rational& x, const big_num_rational& y) {
  return big_num_rational(big_num::gcd(x.numerator, y.numerator), x.denominator*y.denominator/big_pos_int::gcd(x.denominator, y.denominator));
}

big_pos_int big_int::operator%(const big_pos_int& y) const {
    if (*this >= 0) {  
        return static_cast<big_pos_int>(*this) % y;
    } else {
        return static_cast<big_pos_int>(*this+y*abs(*this)) % y;
    }
}
