//
// Created by user on 7/17/21.
//

#ifndef NS_3_29_FSM_H
#define NS_3_29_FSM_H

#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <vector>

namespace ns3 {
namespace aodv {

enum FsmState {
  FSM_WATCHING_HELLO_FLOOD,
  FSM_IDLE,
  FSM_ALERT,
};

inline std::ostream& operator<< (std::ostream& os, FsmState state) {
  switch (state)
  {
  case FSM_IDLE:
    return (os << "FSM_IDLE");
  case FSM_ALERT:
    return (os << "FSM_ALERT");
  case FSM_WATCHING_HELLO_FLOOD:
    return (os << "FSM_WATCHING_HELLO_FLOOD");
  }
}

class Fsm {
public:
  Fsm(Time helloFloodTrackingPeriod, int helloFloodMaxInPeriod, Ipv4Address neighborIp);
  void ProcessHello();
  bool IsMalicious();

  Ipv4Address m_neighborIp;

private:
  void ResetFsmStateToIdle();

  bool m_isMalicious;
  Time m_helloFloodTrackingPeriod;
  int m_helloFloodMaxInPeriod;
  int m_helloPacketsInCurrentPeriod;

  FsmState m_currentState;
  Time m_periodEndTime;
};

class Fsms {
public:
  static Fsm& Get(Ipv4Address neighborIp);

  static std::vector<Fsm> m_fsms;

private:
  static Fsm& Create(Ipv4Address neighborIp);
};

}
}



#endif //NS_3_29_FSM_H
