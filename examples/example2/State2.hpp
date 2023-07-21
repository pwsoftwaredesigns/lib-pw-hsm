#ifndef EXAMPLE2_STATE2_HPP_
#define EXAMPLE2_STATE2_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "Handler.hpp"

namespace example2
{

class State2 : public pw::hsm::State<State2, Handler, StateRoot>
{
public:
	State2(Parent& parent);
	~State2();
	
}; //class State2

} //namespace example2

#endif //EXAMPLE2_STATE2_HPP_