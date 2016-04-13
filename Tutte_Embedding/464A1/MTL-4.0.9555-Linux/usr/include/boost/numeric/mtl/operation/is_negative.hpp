// Software License for MTL
// 
// Copyright (c) 2007 The Trustees of Indiana University. 
//               2008 Dresden University of Technology and the Trustees of Indiana University.
//               2010 SimuNova UG (haftungsbeschränkt), www.simunova.com. 
// All rights reserved.
// Authors: Peter Gottschling and Andrew Lumsdaine
// 
// This file is part of the Matrix Template Library
// 
// See also license.mtl.txt in the distribution.

#ifndef MTL_IS_NEGATIVE_INCLUDE
#define MTL_IS_NEGATIVE_INCLUDE

#include <boost/type_traits/is_unsigned.hpp>
#include <boost/utility/enable_if.hpp>

namespace mtl {

template <typename T>
typename boost::enable_if<boost::is_unsigned<T>, bool>::type
inline is_negative(T) { return false; }

template <typename T>
typename boost::disable_if<boost::is_unsigned<T>, bool>::type
inline is_negative(T x) { return x < 0; }

} // namespace mtl

#endif // MTL_IS_NEGATIVE_INCLUDE
