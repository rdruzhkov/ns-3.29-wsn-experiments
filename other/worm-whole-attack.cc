
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

NS_LOG_COMPONENT_DEFINE("AodvWormWholeAttack");

void PrintNodePosition(ns3::Ptr<ns3::Node> ptr_node, int node_id) {
    Ptr<MobilityModel> mob = ptr_node->GetObject<MobilityModel>();
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;

    printf("Node %d: x=%f, y=%f\n", node_id, x, y);
}

int main(int argc, char** argv) {
    uint32_t nodesInGroup = 5;

    NodeContainer nodesGroup1;
    NodeContainer nodesGroup2;
    NetDeviceContainer nodesGroup1Devices;
    NetDeviceContainer nodesGroup2Devices;
    Ipv4InterfaceContainer nodesGroup1Interfaces;
    Ipv4InterfaceContainer nodesGroup2Interfaces;

    SeedManager::SetSeed(12345);

    nodesGroup1.Create(nodesInGroup);
    nodesGroup2.Create(nodesInGroup);

    MobilityHelper mobility1;
    mobility1.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (50),
                                   "DeltaY", DoubleValue (50),
                                   "GridWidth", UintegerValue(2),
                                   "LayoutType", StringValue ("RowFirst"));
    mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility1.Install(nodesGroup1);

    MobilityHelper mobility2;
    mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (250.0),
                                   "DeltaX", DoubleValue (50),
                                   "DeltaY", DoubleValue (50),
                                   "GridWidth", UintegerValue(2),
                                   "LayoutType", StringValue ("RowFirst"));
    mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility2.Install(nodesGroup2);

    std::cout << "\nGroup 1 nodes positions:\n";
    for (uint32_t i=0; i < nodesInGroup; i++) {
        PrintNodePosition(nodesGroup1.Get(i), i);
    }

    std::cout << "\nGroup 2 nodes positions:\n";
        for (uint32_t i=0; i < nodesInGroup; i++) {
        PrintNodePosition(nodesGroup2.Get(i), i + nodesInGroup);
    }

    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());
    WifiHelper wifi;

    wifi.SetRemoteStationManager (
        "ns3::ConstantRateWifiManager",
        "DataMode",
        StringValue ("OfdmRate6Mbps"),
        "RtsCtsThreshold",
        UintegerValue (0)
    );

    nodesGroup1Devices = wifi.Install (wifiPhy, wifiMac, nodesGroup1);
    nodesGroup2Devices = wifi.Install (wifiPhy, wifiMac, nodesGroup2);

    //wifiPhy.EnablePcapAll (std::string ("aodv"));

    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper (aodv);
    stack.Install (nodesGroup1);
    stack.Install(nodesGroup2);

    Ipv4AddressHelper address;
    address.SetBase ("10.0.1.0", "255.255.255.0");
    nodesGroup1Interfaces = address.Assign (nodesGroup1Devices);
    nodesGroup2Interfaces = address.Assign (nodesGroup2Devices);

    NodeContainer p2pNodes;
    p2pNodes.Add(nodesGroup1.Get(nodesInGroup - 1));
    p2pNodes.Add(nodesGroup2.Get(0));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);

    address.SetBase("10.0.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    //pointToPoint.EnablePcapAll ("second");

    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("my_data/aodv.routes", std::ios::out);
    aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);

    Simulator::Stop (Seconds (10));
    Simulator::Run ();

}