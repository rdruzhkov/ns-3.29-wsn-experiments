
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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WsnEnergyDrainAttacks");

int main(int argc, char** argv) {

    NodeContainer nodes;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;

    uint32_t size = 4;

    // Configure
    SeedManager::SetSeed(12345);

    // Creationg of nodes
    nodes.Create (size);

    // Name nodes
    for (uint32_t i = 0; i < size; ++i)
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
                                   "DeltaX", DoubleValue (51),
                                   "DeltaY", DoubleValue (51),
                                   "GridWidth", UintegerValue(2),
                                   "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (nodes);

    // Create devices
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


    // Installing internet stack
    std::cout << "Installing internet stack...\n";
    AodvHelper aodv;
    // you can configure AODV attributes here using aodv.Set(name, value)
    InternetStackHelper stack;
    stack.SetRoutingHelper (aodv); // has effect on the next Install ()
    stack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);

    // Print routes
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
    aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);

    // Install applications
    std::cout << "Installing applications...\n";
    V4PingHelper ping (interfaces.GetAddress (size - 1));
    ping.SetAttribute ("Verbose", BooleanValue (true));
    ping.SetAttribute ("Interval", TimeValue(Seconds(1)));

    ApplicationContainer p = ping.Install (nodes.Get (0));
    p.Start (Seconds (0));
    p.Stop (Seconds (4) - Seconds (0.001));

    Simulator::Stop (Seconds (5));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}