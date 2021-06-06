
#include "ns3/random-variable-stream.h"
#include "experimental-finite-state-machine.h"


namespace ns3 {

  ExperimentalFiniteStateMachine::ExperimentalFiniteStateMachine ()
  {
  }

  bool ExperimentalFiniteStateMachine::IsNextHopLegal (Ipv4Address ipv4Address)
  {
    static int cntr = 0;

    for (std::string blackListedIpv4Address : this->Ipv4AddressesBlackList)
      {
        if (ipv4Address == Ipv4Address (blackListedIpv4Address.c_str ()))
          {
            return false;
            std::cout << "[MY_TAG] FSM blocked " << blackListedIpv4Address << "\n";
          }
      }

      if (ipv4Address == "10.0.0.2")
      {
        std::cout << "[MY_TAG] FSM spotted 10.0.0.2 (not in black list)\n";
        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
        if (cntr > 1) {
          this->Ipv4AddressesBlackList.push_back("10.0.0.2");
          this->isNode2Listed = true;
          std::cout << "[MY_TAG] FSM added 10.0.0.2 to blacklist\n";
        }
        else {
            cntr++;
          std::cout << "[MY_TAG] FSM didn't added 10.0.0.2 to blacklist\n";
          }
      }

    return true;
  }
}

