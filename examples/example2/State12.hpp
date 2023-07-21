#ifndef EXAMPLE2_STATE12_HPP_
#define EXAMPLE2_STATE12_HPP_

#include <pw/hsm.hpp>
#include "states_fwd.hpp"
#include "Handler.hpp"

namespace example2
{

class State12 : public pw::hsm::State<State12, Handler, State1>
{
public:
	State12(Parent& parent);
	~State12();
	
	HandleResult handle(const Event2& e) override;

}; //class State12

} //namespace example2

#endif //EXAMPLE2_STATE12_HPP_