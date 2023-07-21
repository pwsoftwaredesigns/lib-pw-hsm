#include "State11.hpp"

#include "states.hpp"
#include "cout.hpp"
#include "events.hpp"

namespace example2
{

State11::State11(Parent& parent) : 
	State(parent) 
{ 
	COUT_FUNCTION(); 
}

State11::~State11()
{ 
	COUT_FUNCTION(); 
}

State11::HandleResult State11::handle(const Event1& e)
{
	COUT_EVENT(State11, e);
	return kPass;
}

State11::HandleResult State11::handle(const Event2& e)
{
	COUT_EVENT(State11, e);
	return transition<State12>();
}

}