#include "MyStateMachine.hpp"
#include "events.hpp"

namespace example2
{

void MyStateMachine::run()
{
	dispatch(Event1{10, 20});
	dispatch(Event2{30});
	
	dispatch(Event1{40, 50});
	dispatch(Event2{60});
}

}