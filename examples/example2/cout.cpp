#include "cout.hpp"
#include "events.hpp"

namespace example2
{

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

}