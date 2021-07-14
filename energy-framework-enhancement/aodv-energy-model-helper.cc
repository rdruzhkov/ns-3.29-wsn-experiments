/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "aodv-energy-model-helper.h"
#include "ns3/aodv-energy-model.h"
#include "ns3/aodv-routing-protocol.h"

namespace ns3 {

AodvEnergyModelHelper::AodvEnergyModelHelper ()
{
  m_aodvEnergy.SetTypeId ("ns3::AodvEnergyModel");
  m_depletionCallback.Nullify ();
  m_rechargedCallback.Nullify ();
}

AodvEnergyModelHelper::~AodvEnergyModelHelper ()
{
}

void
AodvEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_aodvEnergy.Set (name, v);
}

void
AodvEnergyModelHelper::SetDepletionCallback (
    AodvEnergyModel::AodvEnergyDepletionCallback callback)
{
  m_depletionCallback = callback;
}

void
AodvEnergyModelHelper::SetRechargedCallback (
    AodvEnergyModel::AodvEnergyRechargedCallback callback)
{
  m_rechargedCallback = callback;
}

Ptr<DeviceEnergyModel>
AodvEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                       Ptr<EnergySource> source) const
{
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);

  Ptr<Node> node = device->GetNode ();
  Ptr<aodv::RoutingProtocol> routingProtocol = node->GetObject<aodv::RoutingProtocol>();
  TypeId typeId = routingProtocol->GetTypeId();
  if (typeId.GetName() != std::string("ns3::aodv::RoutingProtocol")) {
    NS_FATAL_ERROR ("RoutingProtocol type is not ns3::aodv::RoutingProtocol!");
  }

  Ptr<AodvEnergyModel> model = m_aodvEnergy.Create ()->GetObject<AodvEnergyModel> ();
  NS_ASSERT (model != NULL);

  routingProtocol->SetCalculationsStartCallback(
    MakeCallback(&AodvEnergyModel::NotifyCalculating, model)
  );
  routingProtocol->SetCalculationsStopCallback(
      MakeCallback(&AodvEnergyModel::NotifyIdle, model)
  );

  // === Set change state callbacks where necessary

//  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
//  Ptr<WifiPhy> wifiPhy = wifiDevice->GetPhy ();
//  wifiPhy->SetAodvE
  //  nergyModel (model);

  // === Set change state callbacks where necessary

  // add model to device model list in energy source
  source->AppendDeviceEnergyModel (model);
  // set energy source pointer
  model->SetEnergySource (source);

  return model;
}

} // namespace ns3
