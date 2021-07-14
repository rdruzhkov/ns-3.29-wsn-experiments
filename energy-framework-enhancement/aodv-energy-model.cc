
#include <chrono>

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/energy-source.h"
#include "aodv-energy-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AodvEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (AodvEnergyModel);

TypeId
AodvEnergyModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::AodvEnergyModel")
      .SetParent<DeviceEnergyModel> ()
      .SetGroupName ("Energy")
      .AddConstructor<AodvEnergyModel> ()
      .AddTraceSource ("TotalEnergyConsumption",
                       "Total energy consumption of the radio device.",
                       MakeTraceSourceAccessor (&AodvEnergyModel::m_totalEnergyConsumption),
                       "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

AodvEnergyModel::AodvEnergyModel ()
:  m_source (0),
   m_idleCurrentA (0.100),
   m_calculationCurrentA (0.200),
   m_currentState (AodvEnergyModelState::IDLE_),
   m_nPendingChangeState (0)
{
  NS_LOG_FUNCTION (this);
  m_energyDepletionCallback.Nullify ();
  m_energyRechargedCallback.Nullify ();
  m_lastUpdateTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

AodvEnergyModel::~AodvEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
AodvEnergyModel::SetEnergySource (const Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
  m_switchToOffEvent.Cancel ();
  Time durationToOff = GetMaximumTimeInState (m_currentState);
  m_switchToOffEvent = Simulator::Schedule (
      durationToOff,
      &AodvEnergyModel::ChangeState,
      this,
      AodvEnergyModelState::OFF_
  );
}

double
AodvEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);

  auto duration = std::chrono::high_resolution_clock::now().time_since_epoch().count() - m_lastUpdateTime;

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (m_currentState)
  {
  case AodvEnergyModelState::IDLE_:
    energyToDecrease = (duration * m_idleCurrentA * supplyVoltage) / 1e7;
    energyToDecrease = 1.0;
    break;
  case AodvEnergyModelState::CALCULATING_:
    energyToDecrease = (duration * m_calculationCurrentA * supplyVoltage) / 1e7;
    energyToDecrease = 1.0;
    break;
  case AodvEnergyModelState::OFF_:
    energyToDecrease = 0;
    break;
  default:
    NS_FATAL_ERROR ("AodvEnergyModel:Undefined state: " << m_currentState);
  }
  if (energyToDecrease == 0) {
    printf("ALERT: Energy to decrease %f\n", energyToDecrease);
    }


  // notify energy source
  m_source->UpdateEnergySource ();

  return m_totalEnergyConsumption + energyToDecrease;
}

Time
AodvEnergyModel::GetMaximumTimeInState (int state) const
{
  Time remainingTime;
  double remainingEnergy = m_source->GetRemainingEnergy ();
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (state)
  {
  case AodvEnergyModelState::IDLE_:
    remainingTime = NanoSeconds (static_cast<uint64_t> (1e9 * (remainingEnergy / (m_idleCurrentA * supplyVoltage))));
    break;
  case AodvEnergyModelState::CALCULATING_:
    remainingTime = NanoSeconds (static_cast<uint64_t> (1e9 * (remainingEnergy / (m_calculationCurrentA * supplyVoltage))));
    break;
  default:
    NS_FATAL_ERROR ("AodvEnergyModelState: undefined state " << state);
  }
  return remainingTime;
}

void
AodvEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  m_nPendingChangeState++;

  if (m_nPendingChangeState > 1 && newState == AodvEnergyModelState::OFF_)
  {
    SetAodvEnergyModelState ((AodvEnergyModelState) newState);
    m_nPendingChangeState--;
    return;
  }

  if (newState != AodvEnergyModelState::OFF_)
  {
    m_switchToOffEvent.Cancel ();
    Time durationToOff = GetMaximumTimeInState (newState);
    m_switchToOffEvent = Simulator::Schedule (
        durationToOff,
        &AodvEnergyModel::ChangeState,
        this,
        AodvEnergyModelState::OFF_);
  }

  auto duration = std::chrono::system_clock::now().time_since_epoch().count() - m_lastUpdateTime;

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (m_currentState)
  {
  case AodvEnergyModelState::IDLE_:
    energyToDecrease = (duration* m_idleCurrentA * supplyVoltage) / 1e7;
    break;
  case AodvEnergyModelState::CALCULATING_:
    energyToDecrease = (duration* m_calculationCurrentA * supplyVoltage) / 1e7;
    break;
  case AodvEnergyModelState::OFF_:
    energyToDecrease = 0.0;
    break;
  default:
    NS_FATAL_ERROR ("AodvEnergyModel: undefined radio state " << m_currentState);
  }

  if (energyToDecrease == 0) {
    printf("ALERT: Energy to decrease %f\n", energyToDecrease);
  }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;
  NS_ASSERT (m_totalEnergyConsumption <= m_source->GetInitialEnergy ());

  // update last update time stamp
  m_lastUpdateTime = std::chrono::system_clock::now().time_since_epoch().count();

  // notify energy source
  m_source->UpdateEnergySource ();

  if (m_nPendingChangeState <= 1 && m_currentState != AodvEnergyModelState::OFF_)
  {
    // update current state & last update time stamp
    SetAodvEnergyModelState ((AodvEnergyModelState) newState);

    // some debug message
    NS_LOG_DEBUG ("AodvEnergyModel:Total energy consumption is " <<
                  m_totalEnergyConsumption << "J");
  }

  m_nPendingChangeState--;
}

void
AodvEnergyModel::SetAodvEnergyModelState ( AodvEnergyModelState state)
{
  if(state == AodvEnergyModelState::CALCULATING_) {
    statesStack.push(AodvEnergyModelState::CALCULATING_);
  }
  else if (state == AodvEnergyModelState::IDLE_) {
    if(!statesStack.empty()) {
      statesStack.pop();
      state = AodvEnergyModelState::CALCULATING_;
    }
    else {
        printf("ALERT: Stack is empty on idle\n");
      }
  }

  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
  {
  case AodvEnergyModelState::IDLE_:
    stateName = "IDLE_";
    break;
  case AodvEnergyModelState::CALCULATING_:
    stateName = "CALCULATING_";
    break;
  case AodvEnergyModelState::OFF_:
    stateName = "OFF_";
    break;
  }
  NS_LOG_DEBUG ("AodvEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ());
}

void
AodvEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("AodvEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
  {
    m_energyDepletionCallback ();
  }
}

void
AodvEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("AodvEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
  {
    m_energyRechargedCallback ();
  }
}

void
AodvEnergyModel::HandleEnergyChanged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("AodvEnergyModel:Energy is changed!");
  if (m_currentState != AodvEnergyModelState::OFF_)
  {
    m_switchToOffEvent.Cancel ();
    Time durationToOff = GetMaximumTimeInState (m_currentState);
    m_switchToOffEvent = Simulator::Schedule (durationToOff, &AodvEnergyModel::ChangeState, this, AodvEnergyModelState::OFF_);
  }
}

double
AodvEnergyModel::DoGetCurrentA (void) const
{
  switch (m_currentState)
  {
  case AodvEnergyModelState::IDLE_:
    return m_idleCurrentA;
  case AodvEnergyModelState::CALCULATING_:
    return m_calculationCurrentA;
  case AodvEnergyModelState::OFF_:
    return 0.0;
  default:
    NS_FATAL_ERROR ("AodvEnergyModel: undefined radio state " << m_currentState);
  }
}

void AodvEnergyModel::NotifyIdle(void) {
  this->ChangeState(AodvEnergyModelState::IDLE_);
}

void AodvEnergyModel::NotifyCalculating(void) {
  this->ChangeState(AodvEnergyModelState::CALCULATING_);
}

}
