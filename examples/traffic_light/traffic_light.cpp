//==============================================================================
// INCLUDES
//==============================================================================

#include <pw/hsm.hpp>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <csignal>

using namespace std::chrono_literals;
	
//==============================================================================
// HELPER CLASSES
//==============================================================================

/**
* @brief Implementation of a thread-safe, blocking queue with timeout
*/
template<typename T>
class LockingQueue
{
public:
	template <typename U>
    void push(U&& data)
    {
        {
            std::lock_guard<std::mutex> lock(guard);
            queue.push(std::forward<U>(data));
        }
        signal.notify_one();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(guard);
        return queue.empty();
    }

    bool tryWaitAndPop(T& value, std::chrono::milliseconds milli)
    {
        std::unique_lock<std::mutex> lock(guard);
        while (queue.empty())
        {
            signal.wait_for(lock, milli);
            return false;
        }

        value = std::move(queue.front());
        queue.pop();
        return true;
    }

private:
    std::queue<T> queue;
    mutable std::mutex guard;
    std::condition_variable signal;
};

//==============================================================================
// EVENTS
//==============================================================================

namespace traffic_light
{

class EPedestrianButton;
class ETick;
class ETimeout;
class ESigint;

using Handler = pw::hsm::EventHandler<EPedestrianButton, ETick, ETimeout, ESigint>;
	
class ECarDetected : public pw::hsm::Event<ECarDetected, Handler>
{
	
};

class EPedestrianButton : public pw::hsm::Event<EPedestrianButton, Handler>
{
	
};

class ETick : public pw::hsm::Event<ETick, Handler>
{
	
};

class ETimeout : public pw::hsm::Event<ETimeout, Handler>
{
	
};

class ESigint : public pw::hsm::Event<ESigint, Handler>
{
public:
	ESigint(int signum) : _signum(signum) {}
		
	auto signum() const { return _signum; }
	
private:
	int _signum;
};

using AbstractEvent = pw::hsm::AbstractEvent<Handler>;
using Q = LockingQueue<std::unique_ptr<traffic_light::AbstractEvent>>;	

//==============================================================================
// STATE DECLARATIONS
//==============================================================================

class TrafficLight;
class SRoot;
class SRed;
class SYellow;
class SGreen;


/**
* @brief State when the traffic light is green
*/
class SGreen : public pw::hsm::State<SGreen, Handler, SRoot>
{
public:
	SGreen(Parent& parent);
		
	HandleResult handle(const EPedestrianButton& e) override;
	HandleResult handle(const ETimeout& e) override;
	
}; //class SGreen


/**
* @brief State when the traffic light is yellow
*/
class SYellow : public pw::hsm::State<SYellow, Handler, SRoot>
{
public:
	SYellow(Parent& parent);
	
	HandleResult handle(const ETimeout& e) override;
	
}; //class SYellow


/**
* @brief State when the traffic light is red
*/
class SRed : public pw::hsm::State<SRed, Handler, SRoot>
{
public:
	SRed(Parent& parent);
	~SRed();
	
	HandleResult handle(const ETimeout& e) override;
	
}; //class SRed


/**
* @brief Root state of the TrafficLight state machine
*/
class SRoot : public pw::hsm::State<SRoot, Handler, TrafficLight, SRed, SYellow, SGreen>
{
public:
	SRoot(Parent& parent);
		
	void startTimeout(std::chrono::seconds ms);
	
	HandleResult handle(const ETick& e) override;
	HandleResult handle(const ESigint& e) override;

public:
	bool pedCrossing = false;
		
private:
	int _timer;
	
}; //class SRoot

/**
* @brief State machine for a basic traffic light that demonstrates queueing
*        of events
*/
class TrafficLight : public pw::hsm::StateMachine<TrafficLight, SRoot>
{
public:
	/**
	* @brief Create a copy of an event on the heap and add it to the dispatch
	*        queue @ref _q to be dispatched into the state machine once the
	*        current RTC step completes.
	*/
	template <typename E>
	void dispatchLater(const E& e);
	
	/**
	* @brief Start the event processing loop
	*/
	int exec();
	
	/**
	* @brief Queue a ESigint event
	*/
	void sigint(int signum);
	
	/**
	* @brief Cause the event processing loop to exit
	*/
	void stop();
	
private:
	Q _q;
	bool _exit = false;
	
}; //class TrafficLight

//==============================================================================
// STATE METHOD DEFINITIONS
//==============================================================================

SGreen::SGreen(Parent& parent) : State(parent)
{
	std::cout << "\r\033[0;42m          \033[0;49m" << std::flush;
	root().startTimeout(15s);
}
	
SGreen::HandleResult SGreen::handle(const EPedestrianButton& e)
{
	/*
	* If the light is green (i.e., it is letting cars through) and a pedestrian
	* presses the button to cross, this should cause the light to timeout
	* (i.e., transition to red) faster and leave the light red longer.
	*/
	root().startTimeout(2s);
	root().pedCrossing = true;
	
	return kHandled;	
}

SGreen::HandleResult SGreen::handle(const ETimeout& e)
{	
	return transition<SYellow>();
}

//==============================================================================

SYellow::SYellow(Parent& parent) : State(parent)
{
	std::cout << "\r\033[0;43m          \033[0;49m" << std::flush;
	root().startTimeout(2s);
}

SYellow::HandleResult SYellow::handle(const ETimeout& e)
{	
	return transition<SRed>();
}

//==============================================================================

SRed::SRed(Parent& parent) : State(parent)
{
	std::cout << "\r\033[0;41m          \033[0;49m" << std::flush;
		
		
	if (root().pedCrossing)
	{
		//Light stays red longer when a pedestrian is crossing
		root().startTimeout(20s);
	}
	else 
	{
		root().startTimeout(10s);
	}
}

SRed::~SRed()
{
	root().pedCrossing = false;
}

SRed::HandleResult SRed::handle(const ETimeout& e)
{	
	return transition<SGreen>();
}

//==============================================================================

SRoot::SRoot(Parent& parent): 
	State(parent), 
	_timer(-1)
{
	
}

void SRoot::startTimeout(std::chrono::seconds sec)
{
	_timer = sec.count();
}

SRoot::HandleResult SRoot::handle(const ETick& e)
{	
	std::cout << "." << std::flush;
	if (_timer > 0)
	{
		--_timer;
	}
	else if (_timer == 0)
	{
		sm().dispatchLater(ETimeout{});
		_timer = -1;
	}
	
	return kHandled;	
}

SRoot::HandleResult SRoot::handle(const ESigint& e)
{	
	//std::cout << "Unhandled SIGINT(" << e.signum() << ")" << std::endl;
		
	//sm().stop();
	
	sm().dispatchLater(EPedestrianButton{});
		
	return kHandled;
}

//==============================================================================

template <typename E>
void TrafficLight::dispatchLater(const E& e)
{
	_q.push(std::make_unique<E>(e));
}

int TrafficLight::exec()
{
	while(!_exit)
	{
		std::unique_ptr<traffic_light::AbstractEvent> e;
		bool success = _q.tryWaitAndPop(e, 1s);
		if (success)
		{
			dispatch(*e);
		}
		else
		{
			dispatch(ETick{});
		}
	}
	
	return 0;
}

void TrafficLight::sigint(int signum)
{
	_q.push(std::make_unique<ESigint>(signum));
}

void TrafficLight::stop()
{
	_exit = true;
}

} //namespace traffic_light

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

static std::unique_ptr<traffic_light::TrafficLight> gLight;

//==============================================================================
// FUNCTIONS
//==============================================================================

static void sigintHandler(int signum)
{
	if (gLight)
	{
		gLight->sigint(signum);
	}
}

//==============================================================================
// MAIN
//==============================================================================

int main()
{
	gLight = std::make_unique<traffic_light::TrafficLight>();

	signal(SIGINT, sigintHandler);
		
	return gLight->exec();
}