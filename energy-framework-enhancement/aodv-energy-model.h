#ifndef NS_3_29_AODV_ENERGY_MODEL_H
#define NS_3_29_AODV_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/fatal-error.h"

#include <stack>

enum AodvEnergyModelState {
  IDLE_,
  CALCULATING_,
  OFF_
};

inline std::ostream& operator<< (std::ostream& os, AodvEnergyModelState state) {
  switch (state)
  {
    case IDLE_:
      return (os << "IDLE_");
    case CALCULATING_:
      return (os << "CALCULATING_");
    case OFF_:
      return (os << "OFF_");
    default:
      NS_FATAL_ERROR ("Invalid state");
      return (os << "INVALID");
  }
}

namespace ns3 {

class AodvEnergyModel : public DeviceEnergyModel {

public:

  typedef Callback<void> AodvEnergyDepletionCallback;

  typedef Callback<void> AodvEnergyRechargedCallback;

public:

  static TypeId GetTypeId (void);

  AodvEnergyModel ();

  virtual ~AodvEnergyModel ();

  void SetEnergySource (Ptr<EnergySource> source);

  double GetTotalEnergyConsumption (void) const;

  void NotifyIdle(void);

  void NotifyCalculating(void);

  void ChangeState (int newState);

  void HandleEnergyDepletion (void);

  void HandleEnergyRecharged (void);

  void HandleEnergyChanged (void);

  Time GetMaximumTimeInState (int state) const;

  void SetAodvEnergyModelState ( AodvEnergyModelState state);

private:

  double DoGetCurrentA (void) const;

  Ptr<EnergySource> m_source; ///< energy source

  // Member variables for current draw in different states.
  double m_idleCurrentA;
  double m_calculationCurrentA;

  /// This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_totalEnergyConsumption;

  AodvEnergyModelState m_currentState;
  double m_lastUpdateTime;

  uint8_t m_nPendingChangeState; ///< pending state change

  /// Energy depletion callback
  AodvEnergyDepletionCallback m_energyDepletionCallback;

  /// Energy recharged callback
  AodvEnergyRechargedCallback m_energyRechargedCallback;

  EventId m_switchToOffEvent; ///< switch to off event

  std::stack<AodvEnergyModelState> statesStack;
};

} // namespace ns3

#endif //NS_3_29_AODV_ENERGY_MODEL_H
