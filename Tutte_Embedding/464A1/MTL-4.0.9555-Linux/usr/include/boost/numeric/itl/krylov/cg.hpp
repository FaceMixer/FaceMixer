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

#ifndef ITL_CG_INCLUDE
#define ITL_CG_INCLUDE

#include <cmath>
#include <cassert>
#include <iostream>
#include <boost/mpl/bool.hpp>

#include <boost/numeric/mtl/concept/collection.hpp>
#include <boost/numeric/itl/itl_fwd.hpp>
#include <boost/numeric/itl/iteration/basic_iteration.hpp>
#include <boost/numeric/itl/pc/identity.hpp>
#include <boost/numeric/itl/pc/is_identity.hpp>
#include <boost/numeric/mtl/operation/dot.hpp>
#include <boost/numeric/mtl/operation/unary_dot.hpp>
#include <boost/numeric/mtl/operation/conj.hpp>
#include <boost/numeric/mtl/operation/resource.hpp>
#include <boost/numeric/mtl/operation/lazy.hpp>
#include <boost/numeric/mtl/interface/vpt.hpp>

namespace itl {

/// Conjugate Gradients without preconditioning
template < typename LinearOperator, typename HilbertSpaceX, typename HilbertSpaceB, 
	   typename Iteration >
int cg(const LinearOperator& A, HilbertSpaceX& x, const HilbertSpaceB& b, 
       Iteration& iter)
{
    mtl::vampir_trace<7001> tracer;
    using std::abs; using mtl::conj; using mtl::lazy;
    typedef HilbertSpaceX Vector;
    typedef typename mtl::Collection<HilbertSpaceX>::value_type Scalar;
    typedef typename Iteration::real                            Real;

    Scalar rho(0), rho_1(0), alpha(0), alpha_1(0);
    Vector p(resource(x)), q(resource(x)), r(resource(x)), z(resource(x));
  
    r = b - A*x;
    rho = dot(r, r);
    while (! iter.finished(Real(sqrt(abs(rho))))) {
	++iter;
	if (iter.first())
	    p = r;
	else 
	    p = r + (rho / rho_1) * p;	   

	// q = A * p; alpha = rho / dot(p, q);
	(lazy(q)= A * p) || (lazy(alpha_1)= lazy_dot(p, q));
	alpha= rho / alpha_1;
	
	x += alpha * p;
	rho_1 = rho;
	(lazy(r) -= alpha * q) || (lazy(rho) = lazy_unary_dot(r));
    }

    return iter;
}

/// Conjugate Gradients
template < typename LinearOperator, typename HilbertSpaceX, typename HilbertSpaceB, 
	   typename Preconditioner, typename Iteration >
int cg(const LinearOperator& A, HilbertSpaceX& x, const HilbertSpaceB& b, 
       const Preconditioner& L, Iteration& iter)
{
    using pc::is_identity;
    if (is_identity(L))
	return cg(A, x, b, iter);

    mtl::vampir_trace<7002> tracer;
    using std::abs; using mtl::conj; using mtl::lazy;
    typedef HilbertSpaceX Vector;
    typedef typename mtl::Collection<HilbertSpaceX>::value_type Scalar;
    typedef typename Iteration::real                            Real;

    Scalar rho(0), rho_1(0), rr, alpha(0), alpha_1;
    Vector p(resource(x)), q(resource(x)), r(resource(x)), z(resource(x));
  
    r = b - A*x;
    rr = dot(r, r);
    while (! iter.finished(Real(sqrt(abs(rr))))) {
	++iter;
	(lazy(z)= solve(L, r)) || (lazy(rho)= lazy_dot(r, z));

	if (iter.first())
	    p = z;
	else 
	    p = z + (rho / rho_1) * p;
	
	(lazy(q)= A * p) || (lazy(alpha_1)= lazy_dot(p, q));
	alpha= rho / alpha_1;
      
	x += alpha * p;
	rho_1 = rho;
	(lazy(r) -= alpha * q) || (lazy(rr) = lazy_unary_dot(r));
    }
    return iter;
}

/// Conjugate Gradients with ignored right preconditioner to unify interface
template < typename LinearOperator, typename HilbertSpaceX, typename HilbertSpaceB, 
	   typename Preconditioner, typename RightPreconditioner, typename Iteration >
int cg(const LinearOperator& A, HilbertSpaceX& x, const HilbertSpaceB& b, 
       const Preconditioner& L, const RightPreconditioner&, Iteration& iter)
{
    return cg(A, x, b, L, iter);
}

/// Solver class for CG method; right preconditioner ignored (prints warning if not identity)
template < typename LinearOperator, typename Preconditioner, 
	   typename RightPreconditioner>
class cg_solver
{
  public:
    /// Construct solver from a linear operator; generate (left) preconditioner from it
    explicit cg_solver(const LinearOperator& A) : A(A), L(A) 
    {
	if (!pc::static_is_identity<RightPreconditioner>::value)
	    std::cerr << "Right Preconditioner ignored!" << std::endl;
    }

    /// Construct solver from a linear operator and (left) preconditioner
    cg_solver(const LinearOperator& A, const Preconditioner& L) : A(A), L(L) 
    {
	if (!pc::static_is_identity<RightPreconditioner>::value)
	    std::cerr << "Right Preconditioner ignored!" << std::endl;
    }

    /// Solve linear system approximately as specified by \p iter
    template < typename HilbertSpaceB, typename HilbertSpaceX, typename Iteration >
    int solve(const HilbertSpaceB& b, HilbertSpaceX& x, Iteration& iter) const
    {
	return cg(A, x, b, L, iter);
    }

    /// Perform one CG iteration on linear system
    template < typename HilbertSpaceB, typename HilbertSpaceX >
    int solve(const HilbertSpaceB& b, HilbertSpaceX& x) const
    {
	itl::basic_iteration<double> iter(x, 1, 0, 0);
	return solve(b, x, iter);
    }
    
  private:
    const LinearOperator& A;
    Preconditioner        L;
};


} // namespace itl

#endif // ITL_CG_INCLUDE
