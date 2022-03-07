#pragma once

#include "base.hpp"
UTL(SYSTEM_HEADER)


namespace utl {
	
	template <class Tp>
	inline void do_not_optimize(Tp const& value) {
	  asm volatile("" : : "r,m"(value) : "memory");
	}

	template <class Tp>
	inline void do_not_optimize(Tp& value) {
	#if defined(__clang__)
	  asm volatile("" : "+r,m"(value) : : "memory");
	#else
	  asm volatile("" : "+m,r"(value) : : "memory");
	#endif
	}
	
}




