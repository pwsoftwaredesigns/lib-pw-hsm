#include "State12.hpp"

#include "states.hpp"
#include "cout.hpp"
#include "events.hpp"

namespace example2
{

State12::State12(Parent& parent) : 
	State(parent) 
{
	COUT_FUNCTION(); 
}

State12::~State12()
{ 
	COUT_FUNCTION();
}

State12::HandleResult State12::handle(const Event2& e)
{
	COUT_EVENT(State12, e);
	return transition<State2>();
}
	
}