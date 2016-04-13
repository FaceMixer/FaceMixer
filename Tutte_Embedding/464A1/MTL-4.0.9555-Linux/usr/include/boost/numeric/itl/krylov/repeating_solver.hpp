// Software License for MTL
// 
// Copyright (c) 2007 The Trustees of Indiana University. 
//               2008 Dresden University of Technology and the Trustees of Indiana University.
//               2010 SimuNova UG, www.simunova.com. 
// All rights reserved.
// Authors: Peter Gottschling and Andrew Lumsdaine
// 
// This file is part of the Matrix Template Library
// 
// See also tools/license/license.mtl.txt in the distribution.

#ifndef ITL_REPEATING_SOLVER_INCLUDE
#define ITL_REPEATING_SOLVER_INCLUDE

#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>
#include <boost/numeric/itl/itl_fwd.hpp>

namespace itl {

/// Class for calling \tparam N iterations of the given \tparam Solver
/** If \tparam Stored is true then the \tparam Solver object is stored (i.e. possibly copied) here.
    Otherwise it is only referred and passing temporary objects to the constructor will cause errors. **/
template <typename Solver, unsigned N, bool Stored>
class repeating_solver
{
    typedef typename boost::mpl::if_c<Stored, Solver, const Solver&>::type solver_type;
  public:
    explicit repeating_solver(const Solver& s) : s(s) {}

    template <typename Matrix>
    explicit repeating_solver(const Matrix& A) : s(A)
    {
	BOOST_STATIC_ASSERT((Stored)); // if matrix is passed class must own solver
    }

    /// Perform one GMRES iteration on linear system
    template < typename VectorIn, typename VectorOut >
    int solve(const VectorIn& b, VectorOut& x) const
    {
	itl::basic_iteration<double> iter(x, N, 0, 0);
	return s.solve(b, x, iter);
    }

  private:
    solver_type   s;
};

} // namespace itl

#endif // ITL_REPEATING_SOLVER_INCLUDE
