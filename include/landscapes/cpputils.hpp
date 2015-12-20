#ifndef CPPUTILS_HPP
#define CPPUTILS_HPP 1


#include <utility>

///http://stackoverflow.com/a/6175873/586784

template<class Iter>
struct iter_pair_range : std::pair<Iter,Iter> {
    iter_pair_range(std::pair<Iter,Iter> const& x)
    : std::pair<Iter,Iter>(x)
    {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
};

template<class Iter>
inline iter_pair_range<Iter> as_range(std::pair<Iter,Iter> const& x)
{ return iter_pair_range<Iter>(x); }


template<class T>
inline auto ireversed(T& v) -> decltype( as_range(std::make_pair(v.rbegin(), v.rend())) )
{
    return as_range(std::make_pair(v.rbegin(), v.rend()));
}


#endif
