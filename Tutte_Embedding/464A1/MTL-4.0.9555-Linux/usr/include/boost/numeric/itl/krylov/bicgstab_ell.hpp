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

// Written by Jan Bos
// Edited  by Peter Gottschling

#ifndef ITL_BICGSTAB_ELL_INCLUDE
#define ITL_BICGSTAB_ELL_INCLUDE

#include <boost/numeric/itl/itl_fwd.hpp>
#include <boost/numeric/mtl/concept/collection.hpp>
#include <boost/numeric/mtl/vector/dense_vector.hpp>
#include <boost/numeric/mtl/matrix/strict_upper.hpp>
#include <boost/numeric/mtl/matrix/dense2D.hpp>
#include <boost/numeric/mtl/utility/irange.hpp>
#include <boost/numeric/mtl/operation/orth.hpp>
#include <boost/numeric/mtl/utility/exception.hpp>
#include <boost/numeric/linear_algebra/identity.hpp>
#include <boost/numeric/mtl/operation/resource.hpp>
#include <boost/numeric/mtl/operation/size.hpp>
#include <boost/numeric/mtl/interface/vpt.hpp>

namespace itl {

/// Bi-Conjugate Gradient Stabilized(ell) 
template < typename LinearOperator, typename Vector, 
	   typename LeftPreconditioner, typename RightPreconditioner, 
	   typename Iteration >
int bicgstab_ell(const LinearOperator &A, Vector &x, const Vector &b,
		 const LeftPreconditioner &L, const RightPreconditioner &R, 
		 Iteration& iter, size_t l)
{
    mtl::vampir_trace<7006> tracer;
    using mtl::size; using mtl::irange; using mtl::imax; using mtl::mat::strict_upper;
    typedef typename mtl::Collection<Vector>::value_type Scalar;
    typedef typename mtl::Collection<Vector>::size_type  Size;

    if (size(b) == 0) throw mtl::logic_error("empty rhs vector");

    const Scalar                zero= math::zero(Scalar()), one= math::one(Scalar());
    Vector                      x0(resource(x)), y(resource(x));
    mtl::dense_vector<Vector>   r_hat(l+1,Vector(resource(x))), u_hat(l+1,Vector(resource(x)));

    // shift problem 
    x0= zero;
    r_hat[0]= b;
    if (two_norm(x) != zero) {
	r_hat[0]-= A * x;
	x0= x;
	x= zero;
    }

    Vector  r0_tilde(r_hat[0]/two_norm(r_hat[0]));
    y= solve(L, r_hat[0]);
    r_hat[0]= y;
    u_hat[0]= zero;

    Scalar                      rho_0(one), rho_1(zero), alpha(zero), Gamma(zero), beta(zero), omega(one); 
    mtl::mat::dense2D<Scalar>        tau(l+1, l+1);
    mtl::dense_vector<Scalar>   sigma(l+1), gamma(l+1), gamma_a(l+1), gamma_aa(l+1);

    while (! iter.finished(r_hat[0])) {
	++iter;
	rho_0= -omega * rho_0;

	for (Size j= 0; j < l; ++j) {
	    rho_1= dot(r0_tilde, r_hat[j]); 
	    beta= alpha * rho_1/rho_0; rho_0= rho_1;

	    for (Size i= 0; i <= j; ++i)
		u_hat[i]= r_hat[i] - beta * u_hat[i];
      
	    y= A * Vector(solve(R, u_hat[j]));
	    u_hat[j+1]= solve(L, y);
	    Gamma= dot(r0_tilde, u_hat[j+1]); 
	    alpha= rho_0 / Gamma;

	    for (Size i= 0; i <= j; ++i)
		r_hat[i]-= alpha * u_hat[i+1];
      
	    if (iter.finished(r_hat[j])) {
		x= solve(R, x);
		x+= x0;
		return iter;
	    }

	    r_hat[j+1]= solve(R, r_hat[j]);
	    y= A * r_hat[j+1]; 
	    r_hat[j+1]= solve(L, y);
	    x+= alpha * u_hat[0];
	}

	// mod GS (MR part)
	irange  i1m(1, imax);
	mtl::dense_vector<Vector>   r_hat_tail(r_hat[i1m]);
	tau[i1m][i1m]= orthogonalize_factors(r_hat_tail);
	for (Size j= 1; j <= l; ++j) 
	    gamma_a[j]= dot(r_hat[j], r_hat[0]) / tau[j][j];

	gamma[l]= gamma_a[l]; omega= gamma[l];
	if (omega == zero) return iter.fail(3, "bicg breakdown #2");

	// is this something like a tri-solve? 
	for (Size j= l-1; j > 0; --j) {
	    Scalar sum= zero;
	    for (Size i=j+1;i<=l;++i)
		sum += tau[j][i] * gamma[i];
	    gamma[j] = gamma_a[j] - sum;
	}

	gamma_aa[irange(1, l)]= strict_upper(tau[irange(1, l)][irange(1, l)]) * gamma[irange(2, l+1)] + gamma[irange(2, l+1)];

	x+= gamma[1] * r_hat[0];
	r_hat[0]-= gamma_a[l] * r_hat[l];
	u_hat[0]-= gamma[l] * u_hat[l];
	for (Size j=1; j < l; ++j) {
	    u_hat[0] -= gamma[j] * u_hat[j];
	    x+= gamma_aa[j] * r_hat[j];
	    r_hat[0] -= gamma_a[j] * r_hat[j];
	}
    }
    x= solve(R, x); x+= x0; // convert to real solution and undo shift
    return iter;
}


/// Solver class for BiCGStab(ell) method; right preconditioner ignored (prints warning if not identity)
template < typename LinearOperator, typename Preconditioner, 
	   typename RightPreconditioner>
class bicgstab_ell_solver
{
  public:
    /// Construct solver from a linear operator; generate (left) preconditioner from it
    explicit bicgstab_ell_solver(const LinearOperator& A, size_t l= 8) : A(A), l(l), L(A), R(A) {}

    /// Construct solver from a linear operator and left preconditioner
    bicgstab_ell_solver(const LinearOperator& A, size_t l, const Preconditioner& L) : A(A), l(l), L(L), R(A) {}

    /// Construct solver from a linear operator and left preconditioner
    bicgstab_ell_solver(const LinearOperator& A, size_t l, const Preconditioner& L, const RightPreconditioner& R) 
      : A(A), l(l), L(L), R(R) {}

    /// Solve linear system approximately as specified by \p iter
    template < typename HilbertSpaceB, typename HilbertSpaceX, typename Iteration >
    int solve(const HilbertSpaceB& b, HilbertSpaceX& x, Iteration& iter) const
    {
	return bicgstab_ell(A, x, b, L, R, iter, l);
    }

    /// Perform one BiCGStab(ell) iteration on linear system
    template < typename HilbertSpaceB, typename HilbertSpaceX >
    int solve(const HilbertSpaceB& b, HilbertSpaceX& x) const
    {
	itl::basic_iteration<double> iter(x, 1, 0, 0);
	return solve(b, x, iter);
    }
    
  private:
    const LinearOperator& A;
    size_t                l;
    Preconditioner        L;
    RightPreconditioner   R;
};


} // namespace itl

#endif // ITL_BICGSTAB_ELL_INCLUDE
