/*
 * helpers.hpp
 *
 *  Created on: Jan 20, 2015
 *      Author: indictree
 */

#ifndef DEBUG_HELPERS_HPP_
#define DEBUG_HELPERS_HPP_

template <typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& v)
{
	s.put('[');
	for (int i = 0; i < v.size(); i++) {
		if (i != (v.size() -1)) {
			s << v[i] << ", ";
		}
		else {
			s << v[i];
		}
	}
	s.put(']');
	return s;
}

#endif /* DEBUG_HELPERS_HPP_ */
