#include "State1.hpp"

/*
* The definition of State1 requires the declaration (not just the forward
* declaration) of its parent state(s) in order to compile.
*/
#include "states.hpp"
#include "cout.hpp"
#include "events.hpp"

namespace example2
{

State1::State1(Parent& parent) : 
	State(parent) 
{ 
	COUT_FUNCTION(); 
}

State1::~State1() 
{ 
	COUT_FUNCTION(); 
}

State1::HandleResult State1::handle(const Event1& e)
{
	COUT_EVENT(State1, e);
	return pw::hsm::kHandled;
}

State1::HandleResult State1::handle(const Event2& e)
{
	COUT_EVENT(State1, e);
	return transition<State2>();
}

}