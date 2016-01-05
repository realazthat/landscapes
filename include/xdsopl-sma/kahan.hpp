/*
Kahan - Kahan summation
Written in 2015 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef KAHAN_HH
#define KAHAN_HH

template <typename T>
class Kahan
{
	T high, low;
public:
	Kahan() : high(0), low(0) {}
	Kahan(T init) : high(init), low(0) {}
	T operator ()(T input)
	{
		T tmp = input - low;
		T sum = high + tmp;
		low = (sum - high) - tmp;
		return high = sum;
	}
	T operator ()() { return high; }
};

template <class I, class T>
T kahan_sum(I begin, I end, T init)
{
	Kahan<T> kahan(init);
	while (begin != end)
		kahan(*begin++);
	return kahan();
}

#endif

