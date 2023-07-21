/*
* What can an event handler return?
* 1) "Handled", stay in the same state and do not pass the event up the state
*    chain
* 2) "Passed", stay in the same state and pass the event up the state chain
* 3) "Transition", perform a state transition (to a different or same state)
*    do not pass the event up the chain (because the chain has changed)
*/

#include <iostream>
#include <pw/hsm.hpp>

#define COUT_ENTER() std::cout << __FUNCTION__ << std::endl
#define COUT_EXIT() std::cout << __FUNCTION__ << std::endl
#define COUT_EVENT(s_, e_) std::cout << #s_ << "(" #e_ ")" << std::endl	

using namespace pw::hsm;

//==============================================================================

class Event1;
class Event2;

using Handler = EventHandler<Event1, Event2>;

//==============================================================================

class Event1 : public Event<Event1, Handler>
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

class Event2 : public Event<Event2, Handler>
{
public:
	Event2() = default;
	Event2(int a) : _a(a) {}
		
	auto a() const { return _a; }
	
private:
	int _a = 0;
};

//==============================================================================

/*
class SpecialType1
{
public:
	SpecialType1() { COUT_FUNCTION(); }
	~SpecialType1() { COUT_FUNCTION(); }
	
private:
	unsigned int _array[16];
};

class SpecialType2
{
public:
	SpecialType2() { COUT_FUNCTION(); }
	~SpecialType2() { COUT_FUNCTION(); }
	
private:
	int _i;
};
*/

//==============================================================================

namespace sm1
{

class MyStateMachine;
class StateRoot;
class State1;
class State11;
class State12;
class State121;
class State2;

class State11 : public State<State11, Handler, State1>
{
public:
	State11(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State11() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State11, Event1);
		//return kPass;
		return transition<State12>();
	}
	
private:
	//SpecialType1 _data;
};

class State121 : public State<State121, Handler, State12>
{
public:
	State121(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State121() { COUT_EXIT(); }
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State121, Event2);
		return kPass;
	}	
};

class State12 : public State<State12, Handler, State1, State121>
{
public:
	State12(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State12() { COUT_EXIT(); }
	
	HandleResult handle(const Event2& e) override;
	
private:
	//SpecialType2 _data;
};

class State1 : public State<State1, Handler, StateRoot, State11, State12>
{
public:
	State1(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State1() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State1, Event1);
		return kHandled;
	}
};

class State2 : public State<State2, Handler, StateRoot>
{
public:
	State2(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State2() { COUT_EXIT(); }
};

class StateRoot : public State<StateRoot, Handler, MyStateMachine, State1, State2>
{
public:
	StateRoot(Parent& parent) : State(parent) { COUT_ENTER(); }
	~StateRoot() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(StateRoot, Event1);
		return kHandled;
	}
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(StateRoot, Event2);
		return kHandled;
	}
};

class MyStateMachine : public StateMachine<MyStateMachine, StateRoot>
{
	friend class State12;
	
public:
	void test1()
	{
		dispatch(Event1{10, 20});
		dispatch(Event2{30});
	}
};

HandleResult State12::handle(const Event2& e)
{
	//parent()._privateData = 10;
	//parent().protectedData = 20;
	//parent().publicData = 30;
	
	//root()._privateData = 10;
	//root().protectedData = 20;
	//root().publicData = 30;
	
	//sm().protectedData = 40;
	
	COUT_EVENT(State12, Event2);
	//return kPass;
	return transition<State2>();
}

} //namespace sm1

//==============================================================================

namespace sm2
{

class MyStateMachine;
class StateRoot;
class State1;
class State11;
class State12;
class State121;
class State2;

class State11 : public State<State11, Handler, State1>
{
public:
	State11(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State11() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State11, Event1);
		//return kPass;
		return transition<State12>();
	}
	
private:
	//SpecialType1 _data;
};

class State121 : public State<State121, Handler, State12>
{
public:
	State121(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State121() { COUT_EXIT(); }
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State121, Event2);
		return kPass;
	}	
};

class State12 : public State<State12, Handler, State1, State121>
{
public:
	State12(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State12() { COUT_EXIT(); }
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(State12, Event2);
		return transition<State2>();
	}
	
private:
	//SpecialType2 _data;
};

class State1 : public State<State1, Handler, StateRoot, State11, State12>
{
public:
	State1(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State1() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(State1, Event1);
		return kHandled;
	}
};

class State2 : public State<State2, Handler, StateRoot>
{
public:
	State2(Parent& parent) : State(parent) { COUT_ENTER(); }
	~State2() { COUT_EXIT(); }
};

class StateRoot : public State<StateRoot, Handler, MyStateMachine, State1, State2>
{
public:
	StateRoot(Parent& parent) : State(parent) { COUT_ENTER(); }
	~StateRoot() { COUT_EXIT(); }
	
	HandleResult handle(const Event1& e) override
	{
		COUT_EVENT(StateRoot, Event1);
		return kHandled;
	}
	
	HandleResult handle(const Event2& e) override
	{
		COUT_EVENT(StateRoot, Event2);
		return kHandled;
	}
};

class MyStateMachine : public StateMachine<MyStateMachine, StateRoot>
{
	friend class State12;
	
public:
	void test1()
	{
		dispatch(Event1{10, 20});
		dispatch(Event2{30});
	}
};

} //namespace sm2

//==============================================================================

int main()
{
	sm1::MyStateMachine sm1;
	
	sm1.test1();
	
	sm2::MyStateMachine sm2;
		
	sm2.test1();
	
	return 0;
}