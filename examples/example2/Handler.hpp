#ifndef EXAMPLE2_HANDLER_HPP_
#define EXAMPLE2_HANDLER_HPP_

#include <pw/hsm.hpp>

/*
* The handler/visitor base class only requies the forward declarations of each
* event (because it declares virtual methods accepting const references to
* these events).
*/
#include "events_fwd.hpp"

namespace example2
{

using Handler = pw::hsm::EventHandler<Event1, Event2>;

} //namespace example2

#endif //EXAMPLE2_HANDLER_HPP_