/*
pairwise - pairwise reduction accelerator
Written in 2015 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef PAIRWISE_HH
#define PAIRWISE_HH

template <typename TYPE, size_t SIZE, typename OPERATOR>
class Pairwise
{
	std::vector<TYPE> tree;
	size_t node;
	OPERATOR op;
public:
	typedef TYPE value_type;
	// 2 * N and indexing from 1 on purpose: simpler code
	Pairwise() : tree(2 * SIZE), node(1) {}
	TYPE &operator [] (size_t i)
	{
		reduce();
		return tree[node = i + SIZE];
	}
	TYPE operator [] (size_t i) const
	{
		return tree[i + SIZE];
	}
	TYPE reduce() {
		while (node != 1) {
			size_t left = node & ~1;
			size_t right = node | 1;
			tree[node /= 2] = op(tree[left], tree[right]);
		}
		return tree[1];
	}
};

#endif

