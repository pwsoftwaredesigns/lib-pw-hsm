#ifndef EXAMPLE2_STATE11_HPP_
#define EXAMPLE2_STATE11_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "Handler.hpp"

namespace example2
{

class State11 : public pw::hsm::State<State11, Handler, State1>
{
public:
	State11(Parent& parent);
	~State11();
	
	HandleResult handle(const Event1& e) override;
	HandleResult handle(const Event2& e) override;
	
}; //class State11 

} //namespace example2

#endif //EXAMPLE2_STATE1_HPP_