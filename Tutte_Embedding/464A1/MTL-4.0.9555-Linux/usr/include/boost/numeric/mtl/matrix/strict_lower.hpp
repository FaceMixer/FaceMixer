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

#ifndef MTL_MATRIX_STRICT_LOWER_INCLUDE
#define MTL_MATRIX_STRICT_LOWER_INCLUDE

namespace mtl { namespace mat {

namespace traits {

    template <typename Matrix>
    struct strict_lower
    {
	typedef typename traits::bands<Matrix>::type type;
    };
}

///  Strict lower triangular matrix
template <typename Matrix> 
typename traits::strict_lower<Matrix>::type
inline strict_lower(const Matrix& A)
{
    return bands(A, std::numeric_limits<long>::min(), 0);
}

/// Triangle-lower starting at off-diagonoal \p d (for compatibility with matlib)
template <typename Matrix> 
typename traits::strict_lower<Matrix>::type
inline tril(const Matrix& A, long d= 0)
{
    return bands(A, std::numeric_limits<long>::min(), d+1);
}


}} // namespace mtl::matrix

#endif // MTL_MATRIX_STRICT_LOWER_INCLUDE
