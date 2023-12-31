#ifndef INCLUDE_PW_HSM_HPP_
#define INCLUDE_PW_HSM_HPP_

#include <variant>
#include <tuple>

//==============================================================================

namespace pw::hsm::detail
{

/**
* @brief Trait used to find the first type in a parameter pack
*
* See https://stackoverflow.com/questions/45578484/is-it-possible-to-get-the-first-type-of-a-parameter-pack-in-a-one-liner
*/
template <typename ... Ts>
using first_of_t = typename std::tuple_element<0, std::tuple<Ts...>>::type;
	
/**
* @brief Trait class which finds the "nearest" ancestor (child) to state T in 
*        the chain to U
*
* This trait is used during a transition by the least-common-ancestor (LCA) to
* determine which child state it needs to create.
*/
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

} //namespace pw::hsm::detail

//==============================================================================

namespace pw::hsm
{

//using HandleResult = detail::TransitionObject;
using HandleResult = bool;

/**
* @brief Return from within a state event handler to signify that this state
*        has fully "handled" the event and thus it should not be dispatched to
*        the parent state.
*/
inline const auto kHandled = true;

/**
* @brief Return from within a state event handler to signify that this state
*        would like to "pass" the event to its parent (i.e., it has not fully
*        "handled" the event)
*/
inline const auto kPass = false;

} //namespace pw::hsm

//==============================================================================

namespace pw::hsm
{

/**
* @brief Abstract base class for all event's with a given handler 
*        (i.e., visitor)
*
* @tparam HANDLER Typename of the handler/visitor base class
*/
template <typename HANDLER>
class AbstractEvent
{
public:
	virtual HandleResult accept(HANDLER& h) const = 0;
};

/**
* @brief CRTP base class for an event
*
* @tparam T Typename of the type (CRTP)
* @tparam HANDLER Typename of the handler/visitor base class
*/
template <typename T, typename HANDLER>
class Event : public AbstractEvent<HANDLER>
{
public:
	HandleResult accept(HANDLER& h) const final
	{
		return h.handle(static_cast<const T&>(*this));
	}
};

//==============================================================================

/**
* @brief Template class used to declare an event handler base class
*        (i.e., visitor interface) for events in a state machine.
*/
template <typename ... Es>
class EventHandler;

template <typename FIRST>
class EventHandler<FIRST>
{
public:
	virtual HandleResult handle(const FIRST& e) { return kPass; }
};

template <typename FIRST, typename ... REST>
class EventHandler<FIRST, REST...> : public EventHandler<REST...>
{
public:
	using EventHandler<REST...>::handle;
	virtual HandleResult handle(const FIRST& e) { return kPass; }
};

//==============================================================================

/**
* @brief A state with children
*
* The first child in the CHILDREN parameter pack is considered the "initial
* state" for performing the initial transition.
*/
template <typename T, typename HANDLER, typename PARENT, typename ... CHILDREN>
class State : public HANDLER
{
	/*
	* All State classes are made friends of each other so that they may call
	* the private _doTransition method
	*/
	template <typename T_, typename VISITOR_, typename PARENT_, typename ... CHILDREN_>
	friend
	class State;
	
public:
	using InitialState = detail::first_of_t<CHILDREN...>;
	using NoState = std::monostate;
	using Event = AbstractEvent<HANDLER>;
	using Parent = PARENT;
	
	/*
	* "Import" pw::hsm::HandleResult into State's namespace as a convenience
	* to the library user. Now, in the state's declaration, instead of having
	* to type something like
	*     pw::hsm::HandleResult handle(const MyEvent& e) override;
	* The user can type
	*     HandleResult handle(const MyEvent& e) override;
	*/
	using HandleResult = ::pw::hsm::HandleResult;
	
	/*
	* Added as a convenience to the user so they can directly return kPass or
	* kHandled from a state's event handler methods.
	*/
	inline static const auto kHandled = ::pw::hsm::kHandled;
	inline static const auto kPass = ::pw::hsm::kPass;
	
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
	
	/**
	* @return A const reference to this state's parent
	*/	
	const auto& parent() const { return _parent; }
	
	/**
	* @return A const reference to the root state
	*/
	const auto& root() const { return _parent.root(); }
	
	/**
	* @return A const reference to the StateMachine encompassing this state
	*/
	const auto& sm() const { return _parent.sm(); }
	
	auto& parent() { return _parent; }
	auto& root() { return _parent.root(); }
	auto& sm() { return _parent.sm(); }
	
	/**
	* @brief Perform the initial transition into this state's initial state
	*/
	void init()
	{
		//Construct instance of initial state in variant
		_children.template emplace<InitialState>(static_cast<T&>(*this));
		
		/*
		* Since We know that _children holds InitialState (because we just
		* emplaced it above), we can used std::get directly rather than the
		* slower and larger std::visit
		*/
		std::get<InitialState>(_children).init();
	}
	
	/**
	* @brief Exit all child states by transitioning to NoState
	*/
	void deinit()
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
				handled = arg.dispatch(e);
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
			this->template _doTransition<DEST>(); 
			return true;
		}
		else
		{
			return this->_parent.template transition<DEST>(); 
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
			
			using U = typename detail::nearest_ancestor<T, DEST>::type;
					
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
* @brief Specialization of State for state with no children (i.e., a leaf state)
*/
template <typename T, typename HANDLER, typename PARENT>
class State<T, HANDLER, PARENT> : public HANDLER
{
	template <typename T_, typename VISITOR_, typename PARENT_, typename ... CHILDREN_>
	friend
	class State;
	
public:
	using Event = AbstractEvent<HANDLER>;
	using Parent = PARENT;
	using HandleResult = ::pw::hsm::HandleResult;
	
	inline static const auto kHandled = ::pw::hsm::kHandled;
	inline static const auto kPass = ::pw::hsm::kPass;
	
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
		return this->_parent.template transition<DEST>();
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

} //namespace pw::hsm

#endif //INCLUDE_PW_HSM_HPP_
