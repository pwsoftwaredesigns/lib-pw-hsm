/*
* What can an event handler return?
* 1) "Handled", stay in the same state and do not pass the event up the state
*    chain
* 2) "Passed", stay in the same state and pass the event up the state chain
* 3) "Transition", perform a state transition (to a different or same state)
*    do not pass the event up the chain (because the chain has changed)
*/

#include <variant>
#include <iostream>
#include <tuple>
#include <functional>

#define COUT_ENTER() std::cout << __FUNCTION__ << std::endl
#define COUT_EXIT() std::cout << __FUNCTION__ << std::endl
#define COUT_EVENT(s_, e_) std::cout << #s_ << "(" #e_ ")" << std::endl	
	
//==============================================================================

//See https://stackoverflow.com/questions/45578484/is-it-possible-to-get-the-first-type-of-a-parameter-pack-in-a-one-liner
template <typename ... Ts>
using first_of_t = typename std::tuple_element<0, std::tuple<Ts...>>::type;
	
template <typename T, typename U, typename ENABLE = void>
struct nearest_ancestor
{
	using type = typename nearest_ancestor<T, typename U::Parent>::type;
};

template <typename T, typename U>
struct nearest_ancestor<T, U, std::enable_if_t<std::is_same_v<T, typename U::Parent>>>
{
	using type = U;
};

//==============================================================================

using HandleResult = std::function<bool()>;
constexpr auto kHandled = [](){ return true; };
constexpr auto kPass = [](){ return false; };

//==============================================================================

template <typename HANDLER>
class AbstractEvent
{
public:
	virtual HandleResult accept(HANDLER& h) const = 0;
};

template <typename T, typename HANDLER>
class Event : public AbstractEvent<HANDLER>
{
public:
	HandleResult accept(HANDLER& h) const override
	{
		return h.handle(static_cast<const T&>(*this));
	}
};

//==============================================================================

/*
class AbstractState
{
public:
	virtual void init() { }
	virtual void deinit() { }
};
*/

/**
* @brief A state with children
*
* The first child in the CHILDREN parameter pack is considered the "initial
* state" for performing the initial transition.
*/
template <typename T, typename HANDLER, typename PARENT, typename ... CHILDREN>
class State :/* public AbstractState,*/ public HANDLER
{
	/*
	* All State classes are made friends of each other so that they may call
	* the private _doTransition method
	*/
	template <typename T_, typename VISITOR_, typename PARENT_, typename ... CHILDREN_>
	friend
	class State;
	
public:
	using InitialState = first_of_t<CHILDREN...>;
	using NoState = std::monostate;
	using Event = AbstractEvent<HANDLER>;
	using Parent = PARENT;
	
public:
	template <typename ROOT, typename CHILD>
	static constexpr bool root_has_child()
	{
		return ROOT::template has_child<CHILD>();
	}
	
	/**
	* @retval true if this state has CHILD among any of its immediate or
	*         extended children (i.e., its children's children)
	*/
	template <typename CHILD>
	static constexpr bool has_child()
	{
		//Does this state contain CHILD?
		if constexpr ((std::is_same_v<CHILD, CHILDREN> || ...))
		{
			return true;
		}
		else
		{
			//Do any of this state's children contain CHILD?
			return (root_has_child<CHILDREN, CHILD>() || ...);
		}
	}
	
public:
	State(Parent& parent) : _parent(parent) {}
		
	const auto& parent() const { return _parent; }
	const auto& root() const { return _parent.root(); }
	const auto& sm() const { return _parent.sm(); }
	auto& parent() { return _parent; }
	auto& root() { return _parent.root(); }
	auto& sm() { return _parent.sm(); }
	
	/**
	* @brief Perform the initial transition into this state's initial state
	*/
	void init() //override
	{
		//Construct instance of initial state in variant
		_children.template emplace<InitialState>(static_cast<T&>(*this));
		
		/*
		* Since We know that _children holds InitialState (because we just
		* emplaced it above), we can used std::get directly rather than the
		* slower and larger std::visit
		*/
		std::get<InitialState>(_children).init();
		
		/*
		std::visit([](auto&& arg){
			using U = std::decay_t<decltype(arg)>;
			if constexpr (!std::is_same_v<U, NoState>)
			{
				arg.init();
			}
		}, _children);
		*/
	}
	
	/**
	* @brief Exit all child states by transitioning to NoState
	*/
	void deinit() //override
	{
		/*
		* Since we don't know which state is currently "active" (i.e., which
		* alternative is held by the variant), we must use std::visit here
		*/
		std::visit([](auto&& arg){
			using U = std::decay_t<decltype(arg)>;
			if constexpr (!std::is_same_v<U, NoState>)
			{
				arg.deinit();
			}
		}, _children);
		
		_children.template emplace<NoState>();
	}
	
	/**
	* @brief Send an event to this state to be handled
	*
	* Since this state has children, the event will first be sent to the
	* active child state. If the child state does not "handle" the event, then
	* the event will be dispatched to this state.
	*/
	HandleResult dispatch(const Event& e)
	{
		bool handled = false;
		
		//Dispatch to active child state
		std::visit([&handled, &e](auto&& arg){
			using U = std::decay_t<decltype(arg)>;
			if constexpr (!std::is_same_v<U, NoState>)
			{
				//Dispatch to child state and execute return result functor
				handled = arg.dispatch(e)();
			}
		}, _children);

		if (handled)
		{
			return kHandled;
		}
		else
		{
			//Dispatch to self (use visitor pattern)
			return e.accept(*this);
		}
	}
	
	/**
	* @brief Generate a @ref HandleResult to perform a transition from this
	*        state to state @ref DEST
	*/
	template <typename DEST>
	HandleResult transition()
	{
		if constexpr (has_child<DEST>())
		{
			//This state (the least-common-ancestor) will perform the transition
			return [this](){ 
				this->_doTransition<DEST>(); 
				return true; 
			};
		}
		else
		{
			//Let my parent perform the transition
			return [this](){ return this->_parent.template transition<DEST>()(); };
		}
	}
	
private:
	template <typename DEST> 
	void _doTransition()
	{
		if constexpr (std::is_same_v<T, DEST>)
		{
			this->init();
		}
		else
		{
			this->deinit();
			
			using U = typename nearest_ancestor<T, DEST>::type;
					
			//Create my child which is on the path to DEST
			_children.template emplace<U>(static_cast<T&>(*this));
			
			std::get<U>(_children).template _doTransition<DEST>();
		}
	}
	
public:
	Parent& _parent;
	std::variant<NoState, CHILDREN...> _children;
};

/**
* @brief A state with no children (i.e., a leaf state)
*/
template <typename T, typename HANDLER, typename PARENT>
class State<T, HANDLER, PARENT> : /*public AbstractState,*/ public HANDLER
{
	template <typename T_, typename VISITOR_, typename PARENT_, typename ... CHILDREN_>
	friend
	class State;
	
public:
	using Event = AbstractEvent<HANDLER>;
	using Parent = PARENT;
	
public:
	template <typename CHILD>
	inline static constexpr bool has_child() { return false; }
	
public:
	State(Parent& parent) : _parent(parent) {}
		
	const auto& parent() const { return _parent; }
	const auto& root() const { return _parent.root(); }
	const auto& sm() const { return _parent.sm(); }
	auto& parent() { return _parent; }
	auto& root() { return _parent.root(); }
	auto& sm() { return _parent.sm(); }
	
	void init() {}
	void deinit() {}
	
	HandleResult dispatch(const Event& e)
	{
		//Dispatch to self (use visitor pattern)
		return e.accept(*this);
	}
	
	template <typename DEST>
	HandleResult transition()
	{
		//This state has no children, so the parent state needs to transition
		return [this](){ return _parent.template transition<DEST>()(); };
	}
	
private:
	template <typename DEST> 
	void _doTransition()
	{ 
		this->init();
	}	
	
private:
	Parent& _parent;
	
};

template <typename T, typename ROOT>
class StateMachine
{
public:
	using RootState = ROOT;
	using Event = typename RootState::Event;
	using Parent = void;
	
	const auto& root() const { return _root; }
	const auto& sm() const { return static_cast<const T&>(*this); }
	auto& root() { return _root; }
	auto& sm() { return static_cast<T&>(*this); }
	
	StateMachine() : _root(static_cast<T&>(*this))
	{
		//Peform the initial transition into the root state
		_root.init();
	}
	
	~StateMachine()
	{
		//Exit the root state
		_root.deinit();
	}
	
	void dispatch(const Event& e)
	{
		_root.dispatch(e);
	}
		
private:
	RootState _root;
};

//==============================================================================

class Event1;
class Event2;

class Handler
{
public:
	virtual HandleResult handle(const Event1& e) { return kPass; }
	virtual HandleResult handle(const Event2& e) { return kPass; }
};

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
	
protected:
	int protectedData;
};

//==============================================================================

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
	
//==============================================================================

int main()
{
	MyStateMachine sm;
	
	sm.test1();
	
	return 0;
}