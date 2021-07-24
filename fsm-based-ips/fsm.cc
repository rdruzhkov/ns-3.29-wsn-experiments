#include "ns3/log.h"
#include "ns3/simulator.h"

#include "fsm.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AodvRoutingProtocolFsm");

namespace aodv {

Fsm::Fsm(Time helloFloodTrackingPeriod, int helloFloodMaxInPeriod, Ipv4Address neighborIp)
  : m_neighborIp(neighborIp),
      m_isMalicious(false),
    m_helloFloodTrackingPeriod(helloFloodTrackingPeriod),
    m_helloFloodMaxInPeriod(helloFloodMaxInPeriod),
    m_currentState(FsmState::FSM_IDLE)
{}

void Fsm::ProcessHello ()
{
  if (this->m_currentState != FSM_IDLE && this->m_currentState != FSM_ALERT) {
    if (this->m_periodEndTime < Simulator::Now ()) {
      this->m_currentState = FSM_IDLE;
    }
  }

  if (this->m_currentState == FSM_IDLE)
  {
    this->m_currentState = FSM_WATCHING_HELLO_FLOOD;
    this->m_helloPacketsInCurrentPeriod = 1;
    this->m_periodEndTime = Simulator::Now () + this->m_helloFloodTrackingPeriod;
  }
  else if (this->m_currentState == FSM_WATCHING_HELLO_FLOOD)
  {
    this->m_helloPacketsInCurrentPeriod += 1;
    if (this->m_helloPacketsInCurrentPeriod > this->m_helloFloodMaxInPeriod) {
        this->m_currentState = FSM_ALERT;
        std::cout << "Alert " << this->m_neighborIp << std::endl;
    }
  }
}

bool Fsm::IsMalicious ()
{
  return this->m_currentState == FSM_ALERT;
}

Fsm& Fsms::Create(Ipv4Address neighborIp) {
  Fsms::m_fsms.push_back(
      Fsm( Seconds(1), 10, neighborIp )
  );
  return Fsms::m_fsms.back();
}

Fsm& Fsms::Get(Ipv4Address neighborIp) {
  for (auto& fsm : Fsms::m_fsms) {
    if (fsm.m_neighborIp == neighborIp) {
        return fsm;
      }
  }

  return Fsms::Create(neighborIp);
}

std::vector<Fsm> Fsms::m_fsms = std::vector<Fsm>();

} //namespace aodv

} //namespace ns3