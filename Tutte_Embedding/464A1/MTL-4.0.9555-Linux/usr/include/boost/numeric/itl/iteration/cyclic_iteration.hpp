// Software License for MTL
// 
// Copyright (c) 2007 The Trustees of Indiana University.
//               2008 Dresden University of Technology and the Trustees of Indiana University.
//               2010 SimuNova UG (haftungsbeschränkt), www.simunova.com.
// All rights reserved.
// Authors: Peter Gottschling and Andrew Lumsdaine
// 
// This file is part of the Matrix Template Library2
// 
// See also license.mtl.txt in the distribution.

#ifndef ITL_CYCLIC_ITERATION_INCLUDE
#define ITL_CYCLIC_ITERATION_INCLUDE

#include <iostream>
#include <boost/numeric/itl/iteration/basic_iteration.hpp>

namespace itl {

  /// Class for iteration control that cyclically prints residual
  template <class Real, class OStream = std::ostream>
  class cyclic_iteration : public basic_iteration<Real> 
  {
      typedef basic_iteration<Real> super;
      typedef cyclic_iteration self;

      void print_resid()
      {
	  if (!this->my_quite && this->i % cycle == 0)
	      if (multi_print || this->i != last_print) { // Avoid multiple print-outs in same iteration
		  out << "iteration " << this->i << ": resid " << this->resid() 
		      // << " / " << this->norm_r0 << " = " << this->resid() / this->norm_r0 << " (rel. error)"
		      << std::endl;
		  last_print= this->i;
	      }
      }

    public:
  
      template <class Vector>
      cyclic_iteration(const Vector& r0, int max_iter_, Real tol_, Real atol_ = Real(0), int cycle_ = 100,
		       OStream& out = std::cout)
	: super(r0, max_iter_, tol_, atol_), cycle(cycle_), last_print(-1), multi_print(false), out(out)
      {}

      cyclic_iteration(Real r0, int max_iter_, Real tol_, Real atol_ = Real(0), int cycle_ = 100,
		       OStream& out = std::cout)
	: super(r0, max_iter_, tol_, atol_), cycle(cycle_), last_print(-1), multi_print(false), out(out)
      {}
      

      bool finished() { return super::finished(); }

      template <typename T>
      bool finished(const T& r) 
      {
	  bool ret= super::finished(r);
	  print_resid();
	  return ret;
      }

      inline self& operator++() { ++this->i; return *this; }
      
      inline self& operator+=(int n) { this->i+= n; return *this; }

      operator int() const { return error_code(); }

      /// Whether the residual is printed multiple times in iteration
      bool is_multi_print() const { return multi_print; }

      /// Set whether the residual is printed multiple times in iteration
      void set_multi_print(bool m) { multi_print= m; }

      int error_code() const 
      {
	  if (!this->my_suppress)
	      out << "finished! error code = " << this->error << '\n'
		  << this->iterations() << " iterations\n"
		  << this->resid() << " is actual final residual. \n"
		  << this->relresid() << " is actual relative tolerance achieved. \n"
		  << "Relative tol: " << this->rtol_ << "  Absolute tol: " << this->atol_ << '\n'
		  << "Convergence:  " << pow(this->relresid(), 1.0 / double(this->iterations())) << std::endl;
	  return this->error;
      }
    protected:
      int        cycle, last_print;
      bool       multi_print;
      OStream&   out;
  };



} // namespace itl

#endif // ITL_CYCLIC_ITERATION_INCLUDE
