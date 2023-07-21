#include "State2.hpp"

#include "states.hpp"
#include "cout.hpp"
#include "events.hpp"

namespace example2
{

State2::State2(Parent& parent) : 
	State(parent) 
{ 
	COUT_FUNCTION(); 
}

State2::~State2() 
{ 
	COUT_FUNCTION();
}

}