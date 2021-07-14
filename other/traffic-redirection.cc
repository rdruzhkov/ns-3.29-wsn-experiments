
#include <iostream>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/v4ping-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TrafficRedirection");

void PrintNodePosition(ns3::Ptr<ns3::Node> ptr_node, int node_id) {
  Ptr<MobilityModel> mob = ptr_node->GetObject<MobilityModel>();
  double x = mob->GetPosition().x;
  double y = mob->GetPosition().y;

  printf("Node %d: x=%f, y=%f\n", node_id, x, y);
}

int main(int argc, char** argv) {
  LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC);

  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  SeedManager::SetSeed(12345);

  nodes.Create (4);

  // Name nodes
  for (uint32_t i = 0; i < 4; ++i)
  {
    std::ostringstream os;
    os << "node-" << i;
    Names::Add (os.str (), nodes.Get (i));
  }

  // Create static grid
  std::cout << "Creating static grid...\n";
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (100),
                                 "DeltaY", DoubleValue (100),
                                 "GridWidth", UintegerValue(2),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  std::cout << "\nNodes positions:\n";
  for (uint32_t i=0; i < 4; i++) {
    PrintNodePosition(nodes.Get(i), i);
  }

  std::cout << "Creating devices...\n";
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;

  std::cout << "  Setting wifi remote station manager...\n";
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  std::cout << "  Installing wifi on nodes...\n";
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  wifiPhy.EnablePcapAll (std::string ("all"));

  std::cout << "Configuring AODV...\n";

  AodvHelper aodv;

  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  V4PingHelper ping = V4PingHelper (interfaces.GetAddress(3));
  NodeContainer pinger;
  pinger.Add(nodes.Get(0));

  ApplicationContainer app = ping.Install(pinger);
  app.Start(Seconds(2.0));
  app.Stop(Seconds(5.0));

  // Print routes
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
  aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}