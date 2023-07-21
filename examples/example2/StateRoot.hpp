#ifndef EXAMPLE2_STATEROOT_HPP_
#define EXAMPLE2_STATEROOT_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "State1.hpp"
#include "State2.hpp"

namespace example2
{

class StateRoot : public pw::hsm::State<StateRoot, Handler, MyStateMachine, State1, State2>
{
public:
	StateRoot(Parent& parent);
	~StateRoot();
	
	HandleResult handle(const Event1& e) override;
	HandleResult handle(const Event2& e) override;
	
}; //class StateRoot

} //namespace example2

#endif //EXAMPLE2_STATEROOT_HPP_