#ifndef EXAMPLE2_MYSTATEMACHINE_HPP_
#define EXAMPLE2_MYSTATEMACHINE_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "StateRoot.hpp"

namespace example2
{

class MyStateMachine : public pw::hsm::StateMachine<MyStateMachine, StateRoot>
{	
public:
	void run();
	
}; //class MyStateMachine

}

#endif //EXAMPLE2_MYSTATEMACHINE_HPP_