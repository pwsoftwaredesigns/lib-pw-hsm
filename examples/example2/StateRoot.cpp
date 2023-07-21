#include "StateRoot.hpp"

#include "states.hpp"
#include "cout.hpp"
#include "events.hpp"

namespace example2
{

StateRoot::StateRoot(Parent& parent) : State(parent) { COUT_FUNCTION(); }
StateRoot::~StateRoot() { COUT_FUNCTION(); }

StateRoot::HandleResult StateRoot::handle(const Event1& e)
{
	COUT_EVENT(StateRoot, e);
	return kHandled;
}

StateRoot::HandleResult StateRoot::handle(const Event2& e)
{
	COUT_EVENT(StateRoot, e);
	return kHandled;
}

}