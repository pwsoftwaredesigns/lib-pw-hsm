#include <iostream>
#include <memory>

#ifndef TW_STATEMACHINE_CONFIG_ASSERT
#	define TW_STATEMACHINE_CONFIG_ASSERT(...)
#endif

namespace config
{

constexpr int kMaxStatePathDepth = 8;
constexpr int kMaxDeferredEvents = 4;
constexpr int kEventPoolSize = kMaxDeferredEvents + 2;

}; //namespace

//==============================================================================
// MACROS
//==============================================================================

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define MAP_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

//-----[ MACRO: TW_CAST_CSTATE ]------------------------------------------------
#define TW_CAST_STATE(fn_) CState(static_cast<CState::State>(fn_))

//-----[ MACRO: TW_DECLARE_STATE ]----------------------------------------------
/**
 * @brief Declare a state method and its associated meta-data
 */
#define TW_DECLARE_STATE(...) _TW_DECLARE_STATEN(__VA_ARGS__, _TW_DECLARE_STATE3, _TW_DECLARE_STATE2, _TW_DECLARE_STATE1)(__VA_ARGS__)
#define _TW_DECLARE_STATEN(_1, _2, _3, FN_, ...) FN_
#define _TW_DECLARE_STATE1(name_) @Invalid_usage_of_TW_DECLARE_STATE
#define _TW_DECLARE_STATE2(name_, parent_)\
	CState name_(const AbstractEvent& _e);\
    struct name_##_INFO {\
        static constexpr CState parent() { return TW_CAST_STATE(&parent_); }\
        static constexpr CState init() { return TW_CAST_STATE(nullptr); }\
    }
#define _TW_DECLARE_STATE3(name_, parent_, init_)\
    CState name_(const AbstractEvent& _e);\
    struct name_##_INFO {\
        static constexpr CState parent() { return TW_CAST_STATE(&parent_); }\
        static constexpr CState init() { return TW_CAST_STATE(&init_); }\
    }

#define _TW_DECLARE_TOP_STATE(name_, init_)\
	CState name_(const AbstractEvent& _e);\
    struct name_##_INFO {\
        static constexpr CState parent() { return TW_CAST_STATE(nullptr); }\
        static constexpr CState init() { return TW_CAST_STATE(&init_); }\
    }

//-----[ MACRO: TW_BEGIN_STATE ]------------------------------------------------
/**
 * @brief Begin the definition of a state method
 */
#define TW_BEGIN_STATE(name_) AbstractStateMachine::CState name_(const AbstractEvent& _e){\
    switch(_e.id()){\
        case _INIT::ID:{\
            return name_##_INFO::init();\
        }

//-----[ MACRO: TW_END_STATE ]--------------------------------------------------
/**
 * @brief End the definition of a state method
 */
#define TW_END_STATE(name_) } return name_##_INFO::parent(); }

//-----[ MACRO: TW_BEGIN_EVENT_CASE ]-------------------------------------------
/**
 * @brief Begin the handler for a specific event within a state method
 */
#define TW_BEGIN_EVENT_CASE(...) _TW_BEGIN_EVENT_CASEN(_0, ##__VA_ARGS__, _TW_BEGIN_EVENT_CASE1, _TW_BEGIN_EVENT_CASE0)(__VA_ARGS__)

#define _TW_BEGIN_EVENT_CASEN(_0, _1, FN_, ...) FN_

#define _TW_BEGIN_EVENT_CASE0(...) default:{\
    const AbstractEvent& e = _e;\
    (void)e;
    
#define _TW_BEGIN_EVENT_CASE1(event_) case event_::ID:{\
    const auto& e = static_cast<const event_&>(_e);\
    (void)e;

//-----[ MACRO: TW_END_EVENT_CASE ]---------------------------------------------
/**
 * @brief End the handler for a specific event within a state method
 */
#define TW_END_EVENT_CASE(...) break; }

//-----[ MACRO: TW_BEGIN_MULTI_EVENT_CASE ]-------------------------------------
/**
 * @brief Begin a handler for multiple types of events in a single case
 */
#define TW_BEGIN_MULTI_EVENT_CASE(...)\
MAP(_TW_BEGIN_MULTI_EVENT_CASE_CASE, __VA_ARGS__) {\
    const AbstractEvent& e = _e;\
    (void)e;

#define _TW_BEGIN_MULTI_EVENT_CASE_CASE(e_) case e_::ID:

//-----[ MACRO: TW_END_MULTI_EVENT_CASE ]---------------------------------------
/**
 * @brief End a handler for multiple types of events in a single case
 */
#define TW_END_MULTI_EVENT_CASE(...) break; }

//-----[ MACRO: TW_EVENT_HANDLED ]----------------------------------------------
/**
 * @brief Signify that this event has been handled.
 *
 * Signify that an event has been handled by the current leaf state.  A
 * "handled" event will not be "passed" to the parent state(s).
 * This macro is to be used within an event case (i.e., between
 * @ref TW_BEGIN_EVENT_CASE and @ref TW_END_EVENT_CASE.
 */
#define TW_EVENT_HANDLED() return nullptr

//-----[ MACRO: TW_EVENT_PASS ]-------------------------------------------------
/**
 * @brief Signify that this event has NOT been fully handled.
 *
 * Signify that an event has NOT been handled by the current leaf state. This
 * will allow the event to be passed to the parent state(s).
 * This macro is to be used within an event case (i.e., between
 * @ref TW_BEGIN_EVENT_CASE and @ref TW_END_EVENT_CASE.
 */
#define TW_EVENT_PASS() break

//-----[ MACRO: TW_DECLARE_EVENTS ]---------------------------------------------
/**
 * @brief Declare the event types which are handled by this state machine
 *
 * Example Usage:
 *
 * Events for a ball
 *
 * TW_DECLARE_EVENTS(
 * 	(EInflate, unsigned int air),
 * 	(EDeflate, unsigned int air),
 * 	(EBounce)
 * );
 */
#define TW_DECLARE_EVENTS(...)\
enum {\
    _EVENT_MIN = AbstractStateMachine::_EVENT_MAX,\
    MAP_LIST(_TW_DECLARE_EVENT_ENUM, __VA_ARGS__),\
    _EVENT_MAX\
};\
MAP(_TW_DECLARE_EVENT, __VA_ARGS__)

#define _TW_DECLARE_ROOT_EVENTS(...)\
enum {\
    _EVENT_MIN,\
    MAP_LIST(_TW_DECLARE_EVENT_ENUM, __VA_ARGS__),\
    _EVENT_MAX\
};\
MAP(_TW_DECLARE_EVENT, __VA_ARGS__)

#define _TW_DECLARE_EVENT_ENUM(x_) _TW_DECLARE_EVENT_ENUMX x_
#define _TW_DECLARE_EVENT_ENUMX(name_, ...) _##name_

#define _TW_DECLARE_EVENT(x_) _TW_DECLARE_EVENTX x_
#define _TW_DECLARE_EVENTX(name_, ...) using name_ = Event<_##name_,## __VA_ARGS__>;

//-----[ MACRO: TW_INITIAL_TRANSITION ]--------------------------------------------
/**
 * @brief Macro to be used within a StateMachine's constructor to perform
 *        an initial transition into the state machine.
 * @note A static Transition struct is created by this macro to prevent the
 * 		 transition from being built for every call.
 */
#define TW_INITIAL_TRANSITION(state_) \
	transition(TW_CAST_STATE(&state_))

//-----[ MACRO: TW_FINAL_TRANSITION ]----------------------------------------------
/**
 * @brief Macro to be used in a StateMachine subclass' destructor to exit
 *        the state hierarchy
 */
#define TW_FINAL_TRANSITION(class_) \
	transition(TW_CAST_STATE(&class_::_initialState))

//-----[ MACRO: TW_TRANSITION ]-------------------------------------------------
/**
 * @brief Macro which transitions to another state
 * @note A static Transition struct is created by this macro to prevent the
 * 		 transition from being built for every call.
 *
 * This macro may only be used within the body of the state machine (i.e.
 * within a function defined using TW_DEFINE_STATE)
 */
#define TW_TRANSITION(dest_) \
	do {\
		TW_STATEMACHINE_CONFIG_ASSERT(e.id() >= AbstractStateMachine::_EVENT_MAX);\
		transition(TW_CAST_STATE(&dest_));\
	} while(0)


//-----[ MACRO: TW_DECLARE_OVERRIDE_STATE ]-------------------------------------
/**
 * @brief Declare an override of an existing state in a parent class (must be
 *        declared virtual)
 * @param name_ The name of the state to override
 */
#define TW_DECLARE_OVERRIDE_STATE(name_)\
	CState name_(const AbstractEvent& _e) override;\
	inline auto name_##_SUPER(const AbstractEvent& _e) { return Super::name_(_e); }

//-----[ MACRO: TW_BEGIN_OVERRIDE_STATE ]---------------------------------------
/**
 * @brief Begin an overridden state definition
 *
 * This macro is used in the same way as @ref TW_BEGIN_STATE
 */
#define TW_BEGIN_OVERRIDE_STATE(name_)\
	AbstractStateMachine::CState name_(const AbstractEvent& _e){\
		switch(_e.id()){

//-----[ MACRO: TW_END_OVERRIDE_STATE ]---------------------------------------
/**
 * @brief End an overridden state definition
 *
 * This macro is used in the same way as @ref TW_END_STATE
 */
#define TW_END_OVERRIDE_STATE(name_)\
	} return name_##_SUPER(_e); }

//-----[ CLASS: AbstractEvent ]-------------------------------------------------
class AbstractEvent {
public:
    using id_t = unsigned int;

    virtual ~AbstractEvent() = default;

    virtual id_t id() const = 0;
    virtual std::unique_ptr<AbstractEvent> clone() const = 0;
};

//-----[ TEMPLATE CLASS: Event ]------------------------------------------------
template <AbstractEvent::id_t ID_, typename ... ARGS>
class Event : public AbstractEvent{
public:
    //template <typename ... ARGS2>
    //Event(ARGS2&&... args) : _args(std::forward<ARGS2>(args)...) {}

	//See https://mpark.github.io/programming/2014/06/07/beware-of-perfect-forwarding-constructors/
	Event(ARGS... args) : _args(std::move(args)...) {}

    ~Event() = default;

    static constexpr id_t ID = ID_;
    id_t id() const override { return ID; }
    std::unique_ptr<AbstractEvent> clone() const override { return std::make_unique<Event>(*this); }
    
    template <unsigned int INDEX_ = 0>
    inline const auto& arg() const {
		return std::get<INDEX_>(_args);
	}
    
    template <unsigned int INDEX_ = 0>
	inline auto& arg() {
		return std::get<INDEX_>(_args);
	}

private:
    std::tuple<ARGS...> _args;
};

//-----[ CLASS: AbstractStateMachine ]------------------------------------------
class AbstractStateMachine {
public:
    //-----[ CLASS: CState ]----------------------------------------------------
    class CState {
    public:
        using State = CState (AbstractStateMachine::*)(const AbstractEvent&);
        
        constexpr CState(State fn = nullptr) : _fn(fn) {}
        static constexpr CState null() { return CState(); }
        
        inline CState operator()(AbstractStateMachine& context, const AbstractEvent& e) {
            return context.call(_fn, e);
        }
        
        inline operator bool() const { return _fn != nullptr; }
        inline bool operator==(CState const& other) { return _fn == other._fn;}
        inline bool operator!=(CState const& other) { return _fn != other._fn;}

    private:
        State _fn;
    };
    friend class AbstractStateMachine::CState;
    
    //-----[ STRUCT: StatePath ]------------------------------------------------
	/**
	 * @brief Struct which holds a state ancestry path
	 */
	struct StatePath {
		StatePath() { top = -1; }

		CState states[config::kMaxStatePathDepth];
		int top;

		inline operator bool() const { return top != -1; }
	};

	//-----[ STRUCT: Transition ]-----------------------------------------------
	/**
	 * @brief Struct which holds the exit and entry paths required to perform
	 * 		  a state transition
	 */
	struct Transition {
		StatePath exitPath;
		StatePath enterPath;

		inline operator bool() const { return exitPath && enterPath; }
	};
    
protected:
    //----- Methods ------------------------------------------------------------

    AbstractStateMachine();
    
    virtual ~AbstractStateMachine() = default;

    virtual CState call(CState::State fn, const AbstractEvent& e) = 0;
    
    void dispatch(const AbstractEvent& e);
    
    void transition(CState dest);
    
    //----- Events -------------------------------------------------------------
    
    _TW_DECLARE_ROOT_EVENTS(
		(_EMPTY), //!< Used to cause a state to return its parent
		(ENTER), //!< Sent to a state when it is entered from the outside for the first time
		(_INIT), //!< Sent to the state as part of the initial transition
		(EXIT) //!< Sent to the state when it is exited
	)
    
    //----- States -------------------------------------------------------------
    
    _TW_DECLARE_TOP_STATE(top, AbstractStateMachine::_initialState);
    	TW_DECLARE_STATE(_initialState, AbstractStateMachine::top);
    
private:
    //----- Methods ------------------------------------------------------------
    inline CState _trigger(CState state, const AbstractEvent& e) { return (state)(*this, e); }
    
    CState _getInitialSubstate(CState state);
	void _buildTransition(Transition& tran, CState source, CState dest);
    
    //----- Variables ----------------------------------------------------------
    
    CState _currentState;

    bool _isDispatching;
    
}; //class AbstractStateMachine

//-----[ TEMPLATE CLASS: StateMachine ]-----------------------------------------
template <typename T>
class StateMachine : public AbstractStateMachine {
protected:
    CState call(CState::State fn, const AbstractEvent& e) override {
        return (static_cast<T*>(this)->*fn)(e);
    }
};

//==============================================================================
// CONSTRUCTORS AND DESTRUCTORS
//==============================================================================

//-----[ DEFAULT CONSTRUCTOR: AbstractStateMachine ]----------------------------
AbstractStateMachine::AbstractStateMachine():
    _currentState(&AbstractStateMachine::_initialState),
    _isDispatching(false)
{
    
}

//==============================================================================
// STATES
//==============================================================================

//-----[ STATE: AbstractStateMachine::top ]-------------------------------------
TW_BEGIN_STATE(AbstractStateMachine::top)
{
    
}
TW_END_STATE(AbstractStateMachine::top)

//-----[ STATE: AbstractStateMachine::_initialState ]---------------------------
TW_BEGIN_STATE(AbstractStateMachine::_initialState)
{
    
}
TW_END_STATE(AbstractStateMachine::_initialState)

//==============================================================================
// PROTECTED METHODS
//==============================================================================

//-----[ PROTECTED METHOD: AbstractStateMachine::dispatch ]---------------------
void AbstractStateMachine::dispatch(const AbstractEvent& e) {
    TW_STATEMACHINE_CONFIG_ASSERT(!_isDispatching);

    _isDispatching = true;

    TW_STATEMACHINE_CONFIG_ASSERT(_currentState); //Must be in some valid state

    //Loop through the state "tree"
    for (CState sourceState = _currentState; sourceState; sourceState = sourceState(*this, e)) {}

    _isDispatching = false;
}

//-----[ PROTECTED METHOD: transition ]-----------------------------------------
void AbstractStateMachine::transition(CState dest) {
	Transition tran;

	TW_STATEMACHINE_CONFIG_ASSERT(dest != &AbstractStateMachine::top);

	//1st: Build an entry path from LCA of _currentState and dest to dest (if necessary)
	if (!tran) {
		//The path for this transition has not been initialized
		//If this is the initial transition, _currentState will be NULL
		_buildTransition(tran, _currentState, dest);
	}

	/*
	 * 2nd: Send the EXIT event to all states from _currentState up to (but not
	 * including) LCA
	 */
	for (int i = 0; i < tran.exitPath.top; i++) {
		_trigger(tran.exitPath.states[i], EXIT{});
	}

	//3rd: Send the ENTRY event to all states after LCA to dest
	for (int i = 0; i < tran.enterPath.top; i++) {
		_currentState = tran.enterPath.states[i];
		_trigger(_currentState, ENTER{});
	}

	//4th: Enter all initial states
	CState s = dest;
	do {
		s = _getInitialSubstate(s);
		if (s) {
			_currentState = s;
			_trigger(_currentState, ENTER{});
		}
	} while(s);
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

//-----[ PRIVATE METHOD: AbstractStateMachine::_getInitialSubstate ]------------
AbstractStateMachine::CState AbstractStateMachine::_getInitialSubstate(CState state) {
	TW_STATEMACHINE_CONFIG_ASSERT(state);

	//Get superstate of 'state'
	CState superstate = _trigger(state, _EMPTY{});
    
	//Get potential initial child state of "parent"
	CState substate = _trigger(state, _INIT{});

	/*
	 * If triggering _INIT resulted in a return other than
	 * superstate (i.e. the INIT macro was used), then return this value
	 */
	if (substate != superstate) {
		return substate;
	} else {
		return NULL;
	}
}

//-----[ PRIVATE METHOD: _buildTransition ]-------------------------------------
void AbstractStateMachine::_buildTransition(Transition& tran, CState source, CState dest) {
	int i, j, k;
	StatePath sourceToTop;
	StatePath destToTop;
	CState s;

	TW_STATEMACHINE_CONFIG_ASSERT(source);
	TW_STATEMACHINE_CONFIG_ASSERT(dest);
	TW_STATEMACHINE_CONFIG_ASSERT(source != &AbstractStateMachine::top);
	TW_STATEMACHINE_CONFIG_ASSERT(dest != &AbstractStateMachine::top);

	//--------------------------------------------------------------------------

	//Optimization for self-transition
	if (source == dest) {
		//Self transition
		tran.exitPath.states[0] = source;
		tran.exitPath.top = 1;

		tran.enterPath.states[0] = dest;
		tran.enterPath.top = 1;

		return;
	}

	//1st: Compute the complete path to top from source
	/*
	 * Example sourceToTop array contents
	 * [0] source
	 * [1] source's root
	 * [2] source's root's root
	 * [3] top
	 * [4] NULL
	 */
	s = source;
	sourceToTop.top = 0;
	do {
		TW_STATEMACHINE_CONFIG_ASSERT(sourceToTop.top < config::kMaxStatePathDepth);

		sourceToTop.states[sourceToTop.top] = s;

		//Get the superstate of s
		s = _trigger(s, _EMPTY{});

		++sourceToTop.top;
	} while (s); //Loop until s==NULL (i.e. until we have reached the top state)

	//Top-most discovered ancestor must be top
	TW_STATEMACHINE_CONFIG_ASSERT(sourceToTop.states[sourceToTop.top - 1] == &AbstractStateMachine::top);

	//2nd: Compute the complete path to top from dest
	/*
	 * Example destToTop array contents
	 * [0] dest
	 * [1] dest's root
	 * [2] dest's root's root
	 * [3] top
	 * [4] NULL
	 */
	s = dest;
	destToTop.top = 0;
	do {
		TW_STATEMACHINE_CONFIG_ASSERT(destToTop.top < config::kMaxStatePathDepth);

		destToTop.states[destToTop.top] = s;

		//Get the superstate of s
		s = _trigger(s, _EMPTY{});

		++destToTop.top;
	} while (s); //Loop until s==NULL (i.e. until we have reached the top state)

	//Top-most discovered ancestor must be top
	TW_STATEMACHINE_CONFIG_ASSERT(destToTop.states[destToTop.top - 1] == &AbstractStateMachine::top);

	//--------------------------------------------------------------------------

	//3rd: Find the least-common ancestor for both source and dest
	TW_STATEMACHINE_CONFIG_ASSERT(destToTop.states[destToTop.top - 1] == sourceToTop.states[sourceToTop.top - 1]); //Both chains should be starting at ::top state

	j = destToTop.top; //destToTop.states[j] is ::top
	k = sourceToTop.top; //sourceToTop.states[k] is ::top

	/*
	 * Start working backwards from top until the states do not match
	 * j & k must be greater than 1 (instead of 0) because the state at [0]
	 * is the src/dest itself.  The LCA between a state and itself is the
	 * parent of that state.
	 */
	while ((j > 1) && (k > 1) && (destToTop.states[j - 1] == sourceToTop.states[k - 1])) {
		j--;
		k--;
	}

	/*
	 * At this point j and k are the indices of the LCA between destToTop and
	 * sourceToTop
	 */
	TW_STATEMACHINE_CONFIG_ASSERT(destToTop.states[j] == sourceToTop.states[k]);

	//--------------------------------------------------------------------------

	//4th: Build the exit path from source to LCA
	/*
	 * tran.exitPath.states[0] = source
	 * tran.exitPath.states[1] = source's root
	 * tran.exitPath.states[k-1] = state immediately lower than LCA
	 */
	for (i = 0; i < k; i++) {
		tran.exitPath.states[i] = sourceToTop.states[i];
	}
	tran.exitPath.top = i;

	//5th: Build an entry path from LCA to dest
	/*
	 * Array is copied AND reversed
	 *
	 * destToTop[0] = dest
	 * destToTop[j] = LCA
	 *      |
	 *     \ /
	 * tran.enterPath.states[0] = destToTop[j-1]
	 * tran.enterPath.states[j-1] = dest
	 */
	for (i = 0; i < j; i++) {
		tran.enterPath.states[i] = destToTop.states[(j - 1) - i];
	}
	tran.enterPath.top = i;

	//Done
}

//==============================================================================

#define COUT_ENTER() std::cout << __FUNCTION__ << std::endl
#define COUT_EXIT() std::cout << "~" << __FUNCTION__ << std::endl
#define COUT_EVENT(e_) std::cout << __FUNCTION__ << "(" #e_ ")" << std::endl

namespace sm1
{

class MyStateMachine : public StateMachine<MyStateMachine>
{
public:
	MyStateMachine()
	{
		TW_INITIAL_TRANSITION(MyStateMachine::StateRoot);
	}
	~MyStateMachine()
	{
		TW_FINAL_TRANSITION(MyStateMachine);
	}
	
	void test1()
	{
		dispatch(Event1{10, 20});
		dispatch(Event2{30});
	}
	
protected:
	TW_DECLARE_EVENTS(
		(Event1, int, int),
		(Event2, int)
	);
	
	TW_DECLARE_STATE(StateRoot, MyStateMachine::top, MyStateMachine::State1);
		TW_DECLARE_STATE(State1, MyStateMachine::StateRoot, MyStateMachine::State11);
			TW_DECLARE_STATE(State11, MyStateMachine::State1);
			TW_DECLARE_STATE(State12, MyStateMachine::State1, MyStateMachine::State121);
				TW_DECLARE_STATE(State121, MyStateMachine::State12);
		TW_DECLARE_STATE(State2, MyStateMachine::StateRoot);
};

TW_BEGIN_STATE(MyStateMachine::StateRoot)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event1)
	{
		COUT_EVENT(Event1);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event1)
	
	TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::StateRoot)
	
TW_BEGIN_STATE(MyStateMachine::State1)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
}
TW_END_STATE(MyStateMachine::State1)
	
TW_BEGIN_STATE(MyStateMachine::State11)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event1)
	{
		COUT_EVENT(Event1);
		TW_TRANSITION(MyStateMachine::State12);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event1)
}
TW_END_STATE(MyStateMachine::State11)
	
TW_BEGIN_STATE(MyStateMachine::State12)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_TRANSITION(MyStateMachine::State2);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::State12)
	
TW_BEGIN_STATE(MyStateMachine::State121)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_EVENT_PASS();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::State121)
	
TW_BEGIN_STATE(MyStateMachine::State2)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
}
TW_END_STATE(MyStateMachine::State2)
	
} //namespace sm1

//==============================================================================

namespace sm2
{

class MyStateMachine : public StateMachine<MyStateMachine>
{
public:
	MyStateMachine()
	{
		TW_INITIAL_TRANSITION(MyStateMachine::StateRoot);
	}
	~MyStateMachine()
	{
		TW_FINAL_TRANSITION(MyStateMachine);
	}
	
	void test1()
	{
		dispatch(Event1{10, 20});
		dispatch(Event2{30});
	}
	
protected:
	TW_DECLARE_EVENTS(
		(Event1, int, int),
		(Event2, int)
	);
	
	TW_DECLARE_STATE(StateRoot, MyStateMachine::top, MyStateMachine::State1);
		TW_DECLARE_STATE(State1, MyStateMachine::StateRoot, MyStateMachine::State11);
			TW_DECLARE_STATE(State11, MyStateMachine::State1);
			TW_DECLARE_STATE(State12, MyStateMachine::State1, MyStateMachine::State121);
				TW_DECLARE_STATE(State121, MyStateMachine::State12);
		TW_DECLARE_STATE(State2, MyStateMachine::StateRoot);
};

TW_BEGIN_STATE(MyStateMachine::StateRoot)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event1)
	{
		COUT_EVENT(Event1);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event1)
	
	TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::StateRoot)
	
TW_BEGIN_STATE(MyStateMachine::State1)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
}
TW_END_STATE(MyStateMachine::State1)
	
TW_BEGIN_STATE(MyStateMachine::State11)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event1)
	{
		COUT_EVENT(Event1);
		TW_TRANSITION(MyStateMachine::State12);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event1)
}
TW_END_STATE(MyStateMachine::State11)
	
TW_BEGIN_STATE(MyStateMachine::State12)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_TRANSITION(MyStateMachine::State2);
		TW_EVENT_HANDLED();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::State12)
	
TW_BEGIN_STATE(MyStateMachine::State121)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
    
    TW_BEGIN_EVENT_CASE(Event2)
	{
		COUT_EVENT(Event2);
		TW_EVENT_PASS();
	}
	TW_END_EVENT_CASE(Event2)
}
TW_END_STATE(MyStateMachine::State121)
	
TW_BEGIN_STATE(MyStateMachine::State2)
{
	TW_BEGIN_EVENT_CASE(ENTER)
    {
        COUT_ENTER();
    }
    TW_END_EVENT_CASE(ENTER)
    
    TW_BEGIN_EVENT_CASE(EXIT)
    {
        COUT_EXIT();
    }
    TW_END_EVENT_CASE(EXIT)
}
TW_END_STATE(MyStateMachine::State2)
	
} //namespace sm2

int main()
{
	sm1::MyStateMachine sm1;
	
	sm1.test1();
	
	sm2::MyStateMachine sm2;
	
	sm2.test1();
	
	return 0;
}