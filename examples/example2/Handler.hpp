#ifndef EXAMPLE2_HANDLER_HPP_
#define EXAMPLE2_HANDLER_HPP_

#include <pw/hsm.hpp>
#include "events_fwd.hpp"

namespace example2
{

using Handler = pw::hsm::EventHandler<Event1, Event2>;

} //namespace example2

#endif //EXAMPLE2_HANDLER_HPP_