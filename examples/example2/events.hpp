#ifndef EXAMPLE2_EVENTS_HPP_
#define EXAMPLE2_EVENTS_HPP_

#include <pw/hsm.hpp>
#include "Handler.hpp"

namespace example2
{

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

} //namespace example2

#endif //EXAMPLE2_EVENTS_HPP_