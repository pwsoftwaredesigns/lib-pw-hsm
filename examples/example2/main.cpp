#include <pw/hsm.hpp>
#include <iostream>

#include "MyStateMachine.hpp"
#include "events.hpp"

int main()
{
	example2::MyStateMachine sm;
		
	sm.run();
	
	return 0;
}