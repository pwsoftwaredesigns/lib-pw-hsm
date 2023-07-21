#ifndef EXAMPLE2_STATES_HPP_
#define EXAMPLE2_STATES_HPP_

/*
* This file includes the headers (declarations) for every state in the state
* machine. It must be included by every source (.cpp) file for every state.
* This is necessary because the State template needs to have both the
* implementations of childen (for its declaration) and of its parent(s) for
* the implementation of some of its functions.
*/

#include "StateRoot.hpp"
#include "State1.hpp"
#include "State11.hpp"
#include "State12.hpp"
#include "State2.hpp"

#endif //EXAMPLE2_STATES_HPP_