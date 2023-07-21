#include <pw/hsm.hpp>
#include <iostream>

#define COUT_FUNCTION() std::cout << __FUNCTION__ << std::endl;
#define COUT_EVENT(s_, e_) std::cout << "\033[1;32m" << e_ << " -> " #s_ "\033[0;39m" << std::endl

namespace example1
{

/*
* Forward declare all events here so that we can create a "handler" (i.e.,
* visitor) base class from which all states will inherit.
*/
class Event1;
class Event2;

using Handler = pw::hsm::EventHandler<Event1, Event2>;


class Event1 : public pw::hsm::Event<Event1, Handler>
{
public:
	Event1() = default;
	Event1(int a, int b) : _a(a), _b(b) {}
		
	auto a() const { return _a; }
	auto b() const { return _b; }
	
private:
	int _a = 0;
	int _b = 1;
};

class Event2 : public pw::hsm::Event<Event2, Handler>
{
public:
	Event2() = default;
	Event2(int a) : _a(a) {}
		
	auto a() const { return _a; }
	
private:
	int _a = 0;
};

std::ostream& operator<<(std::ostream& os, const Event1& e)
{
	os << "Event1{a=" << e.a() << ", b=" << e.b() << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Event2& e)
{
	os << "Event2{a=" << e.a() << "}";
	return os;
}

/*
* Forward declare the state machine and states here.
* For documentation purposes, it is recommended to write the states in order
* of their parent-child relationships.  It is also wise to add a comment
* showing the state hierarchy here.
*/
class MyStateMachine;
class StateRoot;
class State1;
class State11;
class State12;
class State2;

/*
* MyStateMachine
* |_ StateRoot
*    |_ State1
*       |_ State11
*       |_ State12
*    |_ State2
*/

/*
* Declare the actual state classes below.
* The order in which you declare state classes is important because of the
* architecture of pw::hsm. All child states must be defined before the 
* parent state may be defined.
*
* States are stored using std::variant which must
* have the complete declaration in order to be declared (so that the size is
* known at compile time). This was a design decision to avoid dynamic memory
* allocation making this library suitable for embedded systems.
*/

/*
* In order to declare a new state, you will inherit from the pw::hsm::State
* template.
* pw::hsm::State<
*     [type of this state], 
*     [type of the handler you defined above],
*     [type of the parent state]
*     (type(s) of the children state(s))
* >
*/
class State11 : public pw::hsm::State<State11, Handler, State1>
{
public:
	/*
	* UML statecharts (i.e., hierarchical state machines) introduce the concept
	* of "entry" and "exit" actions. Entry actions are always executed when
	* the state is entered.  Exit actions are always executed as the state is
	* exited (during a transition).
	*
	* In the pw::hsm framework, states are instantiated when a state is entered
	* and destoryed when exited. This means that the constructor and destructors
	* of states are respectively equivalent to entry and exit actions.
	*
	* This was an intentional design decision to make the implementation of
	* state machines no different than developing normal classes.
	*
	* When created, every state receives a reference to its parent state as
	* an argument. This reference must be passed to the base class (State)
	* constructor.
	*/
	State11(Parent& parent) : 
		State(parent) 
	{ 
		COUT_FUNCTION(); 
	}
	
	~State11()
	{ 
		COUT_FUNCTION(); 
	}
	
	/*
	* Every State inherits from the "handler" base class created above.
	* The pw::hsm::EventHandler template creates a class with a virtual
	* "handle" method for each event type in the state machine. If a state needs
	* to do some action ("handle") on an event, it should override the
	* appropriate handle method for that event.
	*/
	pw::hsm::HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State11, e);
		
		/*
		* A state has 3 options when it is done processing an event
		* 1) "handle" the event - The event will not be dispatched to the parent
		* 2) "pass" the event - The event will be dispatched to the parent as well
		* 3) "transition" to a state - The current state will be exited and a
		*    new state will be entered (which may be the same state)
		*
		* Here we return kPass to indicate that the parent (State1) should also
		* handle this event.
		*/
		return kPass;
	}
	
	pw::hsm::HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State11, e);
		
		/*
		* When an event should cause a state transition, the handler method
		* should return the result of transition() as shown.  This is a
		* templated member method where the template argument is the destination
		* state.
		*/
		return transition<State12>();
	}
	
}; //class State11 

class State12 : public pw::hsm::State<State12, Handler, State1>
{
public:
	State12(Parent& parent) : 
		State(parent) 
	{
		COUT_FUNCTION(); 
	}
	~State12()
	{ 
		COUT_FUNCTION();
	}
	
	/*
	* Every state need not "handle" every type of event. If an event does not
	* need to be handled by the state, is should simply not override the
	* "handle" method for that event.
	*/
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State12, e);
		return transition<State2>();
	}
	
}; //class State12

/*
* State1 has two children: State11 and State12
* They are added to the State base class template after the parent state
*/
class State1 : public pw::hsm::State<State1, Handler, StateRoot, State11, State12>
{
public:
	State1(Parent& parent) : State(parent) { COUT_FUNCTION(); }
	~State1() { COUT_FUNCTION(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State1, e);
		return pw::hsm::kHandled;
	}
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State1, e);
		return transition<State2>();
	}
	
}; //class State1

class State2 : public pw::hsm::State<State2, Handler, StateRoot>
{
public:
	State2(Parent& parent) : State(parent) { COUT_FUNCTION(); }
	~State2() { COUT_FUNCTION(); }
	
}; //class State2

class StateRoot : public pw::hsm::State<StateRoot, Handler, MyStateMachine, State1, State2>
{
public:
	StateRoot(Parent& parent) : State(parent) { COUT_FUNCTION(); }
	~StateRoot() { COUT_FUNCTION(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(StateRoot, e);
		return kHandled;
	}
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(StateRoot, e);
		return kHandled;
	}
	
}; //class StateRoot

/*
* After declare all the states for a state machine, it is time to declare
* the state machine itself. The state machine acts as a container for all states
* which performs the initial transition into the root state (and thus any
* initial child states) in its constructor and exits all current states in
* its destructor. This guarantees that a state machine will be cleaned up
* (all destructors called) correctly.
*
* A state machine class should inherit from pw::hsm::StateMachine
* pw::hsm::StateMachine<
*     [the type of the state machine],
*     [the type of the root state]
* >
*/
class MyStateMachine : public pw::hsm::StateMachine<MyStateMachine, StateRoot>
{	
public:
	/*
	* Dispatch a series of events to the state machine
	
	* You should see the following output:
	*
	* StateRoot
	* State1
	* State11
	* Event1{a=10, b=20} -> State11
	* Event2{a=30} -> State11
	* ~State11
	* State12
	* Event1{a=40, b=50} -> State1
	* Event2{a=60} -> State12
	* ~State12
	* ~State1
	* State2
	* ~State2
	* ~StateRoot
	*/
	void run()
	{
		dispatch(Event1{10, 20});
		dispatch(Event2{30});
		
		dispatch(Event1{40, 50});
		dispatch(Event2{60});
	}
	
}; //class MyStateMachine

} //namespace example1

int main()
{
	example1::MyStateMachine sm;
		
	sm.run();
	
	return 0;
}