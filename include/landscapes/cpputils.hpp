#ifndef CPPUTILS_HPP
#define CPPUTILS_HPP 1


#include <utility>
#include <cstddef>


namespace svo{
    
    
template<typename T>
static inline bool overlap(T x1, T x2, T y1, T y2);


template<typename T>
static inline T ifloor(T value, T modulo);
template<typename T>
static inline T iceil(T value, T modulo);
template<typename T>
static inline T ilog2(T value);


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

///modular decrement.
static inline std::size_t moddec(size_t x0, size_t modulus)
{
    return (x0 + modulus - 1) % modulus;
}
///modular increment.
static inline std::size_t modinc(size_t x0, size_t modulus)
{
    return (x0 + 1) % modulus;
}




template<typename T>
static inline bool overlap(T x1, T x2, T y1, T y2)
{
    ///from http://stackoverflow.com/a/3269471/586784
    assert(x1 <= x2);
    assert(y1 <= y2);

    return x1 <= y2 && y1 <= x2;
}



template<typename T>
static inline T ifloor(T value, T modulo)
{
    return (value / modulo) * modulo;
}

template<typename T>
static inline T iceil(T value, T modulo)
{
    T value1 = ifloor(value, modulo);
    if (value1 != value)
        value1 += modulo;
    
    return value1;
}

template<typename T>
static inline T ilog2(T value)
{
    std::size_t result = 0;

    while (value)
    {
        value >>= 1;
        result += 1;
    }

    return result;
}

} // namespace svo

#endif
