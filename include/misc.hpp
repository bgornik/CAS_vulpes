
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <functional>
#include <initializer_list>

// "assert"-like class
// usage: check(expression) << "error!" -- does nothing if expression if true; if false outputs "error!" and quits program
class check {
    public:
      bool condition;
      std::stringstream out_text;

      check(bool c) : condition(!c) {}

      template<class T>
      check& operator<<(const T& s) {
          if (condition) {
              out_text << s;
          }
          return *this;
      }

      ~check() noexcept(false) {
          if (condition) {
              std::cout << out_text.str();
              throw out_text.str();
          }
      }
};


// "bruteforce" map; only needs equal(usually ==) to be defined for key
template<class K, class T>
class map_bf {
  public:
    std::vector<std::pair<const K,T>> key_val;

    map_bf() : equal([](const K& a, const K& b){ return a==b;  }) {}

    map_bf(std::vector<std::pair<const K,T>>& a) : key_val(a), equal([](const K& a, const K& b){ return a==b; }) {}

    map_bf(const std::function<bool(const K&, const K&)>& f) : equal(f) {}

    map_bf(std::initializer_list<std::pair<const K, T>> init) : equal([](const K& a, const K& b){ return a==b; }) {
        for(auto x : init) {
            key_val.push_back(x);
        }
    }

    class iterator {
      public:
        int i;
        std::vector<std::pair<const K,T>>* key_val;

        iterator(int j, std::vector<std::pair<const K,T>>* kv) : i(j), key_val(kv) {}

        bool operator!=(const iterator& b) const {
            return i!=b.i;
        }

        iterator& operator++() {
            ++i;
            return *this;
        }

        std::pair<const K,T>& operator*() {
            return (*key_val)[i];
        }

        std::pair<const K,T>* operator->() {
            return &((*key_val)[i]);
        }

        bool operator==(const iterator& b) {
            return i==b.i;
        }
    };

    const K at(int i) const {
        check(i < key_val.size()) << "map_bf accesed with too high index\n";
        return key_val[i].first;
    }

    T& operator[](const K& key) {
        int i = -1;
        for(int j = 0; j < key_val.size(); ++j) {
            if (equal(key_val[j].first,key)) { i = j; break; }
        }
        if (i==-1) {
            key_val.push_back(std::make_pair(key, T()));
            i = key_val.size()-1;
        }
        return key_val[i].second;
    }

    iterator begin()  {
        return iterator(0, &key_val);
    }

    iterator end()  {
        return iterator(key_val.size(), &key_val);
    }

    size_t size() const {
        return key_val.size();
    }

    iterator find(const K& key) {
        for(int j = 0; j < key_val.size(); ++j) {
            if (equal(key_val[j].first,key)) {
                return iterator(j, &key_val);
            }
        }
        return iterator(key_val.size(), &key_val);
    }

    bool exists(const K& key) {
        return end()!=find(key);
    }

    bool empty() const {
        return (size()==0);
    }

    private:
      std::function<bool(const K&,const K&)> equal;
};



