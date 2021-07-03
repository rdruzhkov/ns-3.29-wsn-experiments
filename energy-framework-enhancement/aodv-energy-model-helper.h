#ifndef NS_3_29_AODV_ENERGY_MODEL_HELPER_H
#define NS_3_29_AODV_ENERGY_MODEL_HELPER_H

#include "ns3/energy-model-helper.h"
#include "ns3/aodv-energy-model.h"

namespace ns3 {

class AodvEnergyModelHelper : public DeviceEnergyModelHelper
{
public:
  AodvEnergyModelHelper ();

  ~AodvEnergyModelHelper ();

  void Set (std::string name, const AttributeValue &v);

  void SetDepletionCallback (AodvEnergyModel::AodvEnergyDepletionCallback callback);

  void SetRechargedCallback (AodvEnergyModel::AodvEnergyRechargedCallback callback);

private:
  virtual Ptr<DeviceEnergyModel> DoInstall (Ptr<NetDevice> device, Ptr<EnergySource> source) const;

private:
  ObjectFactory m_aodvEnergy; ///< radio energy
  AodvEnergyModel::AodvEnergyDepletionCallback
      m_depletionCallback; ///< radio energy depletion callback
  AodvEnergyModel::AodvEnergyRechargedCallback
      m_rechargedCallback; ///< radio energy recharged callback
};

}

#endif //NS_3_29_AODV_ENERGY_MODEL_HELPER_H
