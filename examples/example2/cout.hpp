#ifndef EXAMPLE2_COUT_HPP_
#define EXAMPLE2_COUT_HPP_

#include <iostream>
#include "events_fwd.hpp"

#define COUT_FUNCTION() std::cout << __FUNCTION__ << std::endl;
#define COUT_EVENT(s_, e_) std::cout << "\033[1;32m" << e_ << " -> " #s_ "\033[0;39m" << std::endl
	
namespace example2
{
	
std::ostream& operator<<(std::ostream& os, const Event1& e);
std::ostream& operator<<(std::ostream& os, const Event2& e);
	
}

#endif //EXAMPLE2_COUT_HPP_