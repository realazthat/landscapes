/*
sma - simple moving average from https://github.com/xdsopl/sma/tree/master
Originally written in 2015 by <Ahmet Inan> <xdsopl@googlemail.com>
Modified in 2015 by <Azriel Fasten> <azriel.fasten@gmail.com>

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
#ifndef SMA_HPP
#define SMA_HPP 1

#include "xdsopl-sma/kahan.hpp"
#include "xdsopl-sma/pairwise.hpp"

template <typename T, int N>
class SMA4
{
    struct add {
        T operator () (T l, T r) { return l + r; }
    };
    Pairwise<T, N, add> history;
    int position;
public:
    SMA4() : position(0) {}
    T operator ()(T v)
    {
        history[position] = v;
        position = (position + 1) % N;
        return history.reduce() / T(N);
    }
};







#endif 
