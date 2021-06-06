
#ifndef NS_3_29_EXPERIMENTAL_FINITE_STATE_MACHINE_H
#define NS_3_29_EXPERIMENTAL_FINITE_STATE_MACHINE_H

#include <vector>
#include <string>
#include "ns3/ipv4-address.h"


namespace ns3 {

class ExperimentalFiniteStateMachine {

public:
    ExperimentalFiniteStateMachine();

    bool IsNextHopLegal(Ipv4Address ipv4Address);

private:
    std::vector<std::string> Ipv4AddressesBlackList;
    bool isNode2Listed = false;
};


}


#endif //NS_3_29_EXPERIMENTAL_FINITE_STATE_MACHINE_H

