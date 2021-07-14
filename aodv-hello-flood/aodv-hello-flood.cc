
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
#include "ns3/energy-module.h"
#include "ns3/wifi-radio-energy-model-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WsnEnergyDrainAttacks");


void PrintNodePosition(ns3::Ptr<ns3::Node> ptr_node, int node_id) {
    Ptr<MobilityModel> mob = ptr_node->GetObject<MobilityModel>();
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;

    printf("Node %d: x=%f, y=%f\n", node_id, x, y);
}


int main(int argc, char** argv) {

    NodeContainer nodes;
    NodeContainer malicious_nodes;
    NetDeviceContainer devices;
    NetDeviceContainer devices_malicious;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer malicious_interfaces;

    uint32_t nodes_quantity = 10;
    uint32_t malicious_nodes_quantity = 1;

    // Configure
    SeedManager::SetSeed(12345);

    // Creationg of nodes
    nodes.Create (nodes_quantity);
    malicious_nodes.Create(malicious_nodes_quantity);

    // Name nodes
    for (uint32_t i = 0; i < nodes_quantity; ++i)
    {
        std::ostringstream os;
        os << "node-" << i;
        Names::Add (os.str (), nodes.Get (i));
    }
    for (uint32_t i = 0; i < malicious_nodes_quantity; ++i) {
        std::ostringstream os;
        os << "malicious-node-" << i;
        Names::Add (os.str (), malicious_nodes.Get (i));
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
    mobility.Install (malicious_nodes);

    std::cout << "\nNormal nodes positions:\n";
    for (uint32_t i=0; i < nodes_quantity; i++) {
        PrintNodePosition(nodes.Get(i), i);
    }
    std::cout << "\nMalicious nodes positions:\n";
    for (uint32_t i=0; i < malicious_nodes_quantity; i++) {
        PrintNodePosition(malicious_nodes.Get(i), i);
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
    devices_malicious = wifi.Install (wifiPhy, wifiMac, malicious_nodes);

    wifiPhy.EnablePcapAll (std::string ("aodv"));

    // Adding energy framework
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (1000));

    EnergySourceContainer sources = basicSourceHelper.Install (nodes);
    EnergySourceContainer malicious_nodes_sources = basicSourceHelper.Install(malicious_nodes);

    WifiRadioEnergyModelHelper radioEnergyHelper;
    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);
    DeviceEnergyModelContainer maliciousDeviceModels = radioEnergyHelper.Install(devices_malicious, malicious_nodes_sources);

    std::cout << "Configuring AODV...\n";

    AodvHelper aodv;

    AodvHelper aodv_hello_flood;
    // Variate hello flood interval to simulate attack of different volume
    aodv_hello_flood.Set(std::string("HelloInterval"), ns3::TimeValue(ns3::Seconds(1)));
    aodv_hello_flood.Set(std::string("EnableHello"), ns3::BooleanValue(true));

    std::cout << "Installing internet stack...\n";

    InternetStackHelper stack;
    stack.SetRoutingHelper (aodv);
    stack.Install (nodes);

    InternetStackHelper malicious_stack;
    malicious_stack.SetRoutingHelper (aodv_hello_flood);
    malicious_stack.Install(malicious_nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);
    malicious_interfaces = address.Assign (devices_malicious);

    // Print routes
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
    aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);

    Simulator::Stop (Seconds (10));
    Simulator::Run ();

    std::cout << "\nEnergy consumption of common nodes:\n";
    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin (); iter != deviceModels.End (); iter ++)
    {
        double energyConsumed = (*iter)->GetTotalEnergyConsumption ();
        NS_LOG_UNCOND ("End of simulation (" << Simulator::Now ().GetSeconds ()
                                             << "s) Total energy consumed by radio = " << energyConsumed << "J");
    }

    std::cout << "\nEnergy consumption of malicious nodes:\n";
    for (DeviceEnergyModelContainer::Iterator iter = maliciousDeviceModels.Begin (); iter != maliciousDeviceModels.End (); iter ++)
    {
        double energyConsumed = (*iter)->GetTotalEnergyConsumption ();
        NS_LOG_UNCOND ("End of simulation (" << Simulator::Now ().GetSeconds ()
                                             << "s) Total energy consumed by radio = " << energyConsumed << "J");
    }

    Simulator::Destroy ();

    return 0;
}