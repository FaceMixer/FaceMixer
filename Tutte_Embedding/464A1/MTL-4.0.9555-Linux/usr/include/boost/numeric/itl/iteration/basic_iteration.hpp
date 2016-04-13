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

#ifndef ITL_BASIC_ITERATION_INCLUDE
#define ITL_BASIC_ITERATION_INCLUDE

#include <iostream>
#include <complex>
#include <string>

namespace itl {


template <class Real>
class basic_iteration
{
  public:
    typedef basic_iteration self;
    typedef Real            real;

    template <class Vector>
    basic_iteration(const Vector& r0, int max_iter_, Real t, Real a = Real(0))
      : error(0), i(0), norm_r0(std::abs(two_norm(r0))),
	max_iter(max_iter_), rtol_(t), atol_(a), is_finished(false), my_quite(false), my_suppress(false) { }

    basic_iteration(Real nb, int max_iter_, Real t, Real a = Real(0))
      : error(0), i(0), norm_r0(nb), max_iter(max_iter_), rtol_(t), atol_(a), is_finished(false), 
	my_quite(false), my_suppress(false) {}

    virtual ~basic_iteration() {}

    bool check_max()
    {
	if (i >= max_iter) 
	    error= 1, is_finished= true, err_msg= "Too many iterations.";
	return is_finished;
    }

    template <class Vector>
    bool finished(const Vector& r) 
    {
	if (converged(two_norm(r)))
	    return is_finished= true;
	return check_max();
    }

    bool finished(const Real& r) 
    {
	if (converged(r))
	    return is_finished= true;
	return check_max();
    }

    template <typename T>
    bool finished(const std::complex<T>& r) 
    {
	if (converged(std::abs(r))) 
	    return is_finished= true;
	return check_max();
    }

    bool finished() const { return is_finished; }

    template <class T>
    int terminate(const T& r) { finished(r); return error; }

    bool converged(const Real& r) { resid_= r; return converged(); }

    bool converged() const 
    { 
	if (norm_r0 == 0)
	    return resid_ <= atol_;  // ignore relative tolerance if |r0| is zero
	return resid_ <= rtol_ * norm_r0 || resid_ <= atol_;
    }

    self& operator++() { ++i; return *this; }

    self& operator+=(int n) { i+= n; return *this; }

    bool first() const { return i <= 1; }

    virtual operator int() const { return error; }

    virtual int error_code() const { return error; }

    bool is_converged() const { return is_finished && error == 0; }

    int iterations() const { return i; }
    
    int max_iterations() const { return max_iter; }

    void set_max_iterations(int m) { max_iter= m; }

    Real resid() const { return resid_; }

    Real relresid() const { return resid_ / norm_r0; }

    Real normb() const { return norm_r0; }

    Real tol() const { return rtol_; }
    Real atol() const { return atol_; }

    int fail(int err_code) { error = err_code; return error_code(); }

    int fail(int err_code, const std::string& msg)
    { error = err_code; err_msg = msg; return error_code(); }

    void set(Real v) { norm_r0 = v; }

    void set_quite(bool q) { my_quite= q; }

    bool is_quite() const { return my_quite; }

    void suppress_resume(bool s) { my_suppress= s; }

    bool resume_suppressed() const { return my_suppress; }

    void update_progress(const basic_iteration& that)
    {
	i= that.i;
	resid_= that.resid_;
	if (that.error > 1) { // copy error except too many iterations
	    error= that.error;
	    err_msg= that.err_msg;
	    is_finished= true;
	} else 
	    finished(resid_);
    }

  protected:
    int          error, i;
    Real         norm_r0;
    int          max_iter;
    Real         rtol_, atol_, resid_;
    std::string  err_msg;
    bool         is_finished, my_quite, my_suppress;
};


} // namespace itl

#endif // ITL_BASIC_ITERATION_INCLUDE
