#include <iostream>
#include <sstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SubnetSimulation");

int main (int argc, char *argv[])
{
    // Log configuration to see progress in terminal
    LogComponentEnable ("SubnetSimulation", LOG_LEVEL_INFO);

    uint32_t nSubnets = 5;
    uint32_t nHosts = 10;

    NodeContainer allNodes; 
    InternetStackHelper stack;
    Ipv4AddressHelper address;
    PointToPointHelper p2p;

    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    // FIX 1: Upgraded mask from /28 (240) to /27 (224) to support up to 30 IPs per group
    Ipv4Mask subnetMask("255.255.255.224");

    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        NodeContainer subnetNodes;
        subnetNodes.Create (nHosts);
        allNodes.Add(subnetNodes);
        stack.Install (subnetNodes);

        // --- Mobility/Positioning ---
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        for (uint32_t j = 0; j < nHosts; ++j) {
            // Space nodes out: X = host index, Y = subnet index
            positionAlloc->Add (Vector (j * 20.0, i * 30.0, 0.0)); 
        }
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (subnetNodes);

        // --- IP Address Assignment ---
        std::stringstream ss;
        // FIX 2: Stride by 32 to ensure the /27 subnets do not overlap
        ss << "192.168.72." << (i * 32);
        address.SetBase (ss.str ().c_str (), subnetMask);

        for (uint32_t j = 0; j < nHosts - 1; ++j) {
            NetDeviceContainer d = p2p.Install (subnetNodes.Get(j), subnetNodes.Get(j+1));
            address.Assign (d);
        }
    }

    // Good practice: Populate routing tables so nodes know about each other 
    // if you decide to add Ping or UDP applications later.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // --- NetAnim Configuration ---
    AnimationInterface anim ("subnet_simulation.xml"); 

    for (uint32_t i = 0; i < allNodes.GetN(); ++i) {
        std::stringstream nodeLabel;
        nodeLabel << "H-" << i;
        anim.UpdateNodeDescription(allNodes.Get(i), nodeLabel.str());
    }

    NS_LOG_INFO ("Starting simulation...");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done. Open subnet_simulation.xml in NetAnim.");

    return 0;
}
