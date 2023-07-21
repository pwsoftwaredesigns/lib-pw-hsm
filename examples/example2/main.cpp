#include <pw/hsm.hpp>
#include "MyStateMachine.hpp"

int main()
{
	example2::MyStateMachine sm;
		
	sm.run();
	
	return 0;
}