#ifndef EXAMPLE2_STATE1_HPP_
#define EXAMPLE2_STATE1_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "Handler.hpp"

/*
* State1 requires the complete declaration of all its child states in order
* to compile correctly. This is because the State template uses std::variant
* to store/switch between active states. Since std::variant stores its objects
* directly (i.e., no dynamic memory allocation) it needs to have the complete
* declaraction of all its types.
*/
#include "State11.hpp"
#include "State12.hpp"

namespace example2
{

class State1 : public pw::hsm::State<State1, Handler, StateRoot, State11, State12>
{
public:
	State1(Parent& parent);
	~State1();
	
	HandleResult handle(const Event1& e) override;
	HandleResult handle(const Event2& e) override;
	
}; //class State1

} //namespace example2

#endif //EXAMPLE2_STATE1_HPP_