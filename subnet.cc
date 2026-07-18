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
    LogComponentEnable ("SubnetSimulation", LOG_LEVEL_INFO);

    // CRITICAL FIX: Enable IP Forwarding globally so the intermediate nodes 
    // in the daisy-chain are allowed to route traffic to the next hop.
    Config::SetDefault("ns3::Ipv4::IpForward", BooleanValue(true));

    uint32_t nSubnets = 5;
    uint32_t nHosts = 10;

    NodeContainer allNodes; 
    InternetStackHelper stack;
    Ipv4AddressHelper address;
    PointToPointHelper p2p;

    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    // We use a global counter to ensure every single link gets a unique subnet
    uint32_t linkSubnetCounter = 1;

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
            positionAlloc->Add (Vector (j * 20.0, i * 30.0, 0.0)); 
        }
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (subnetNodes);

        // --- IP Address Assignment ---
        for (uint32_t j = 0; j < nHosts - 1; ++j) {
            NetDeviceContainer d = p2p.Install (subnetNodes.Get(j), subnetNodes.Get(j+1));
            
            // Create a unique /30 subnet for EVERY single link
            std::stringstream ss;
            ss << "10.1." << linkSubnetCounter++ << ".0";
            
            // Safely cast to Ipv4Address and use a /30 mask (exactly 2 usable IPs for a P2P link)
            address.SetBase (Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.252")); 
            address.Assign (d);
        }
    }

    // Now the routing helper can perfectly map out the network
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
