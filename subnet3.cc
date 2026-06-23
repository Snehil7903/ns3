#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

// Added missing C++ standard library headers
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

int main (int argc, char *argv[])
{
    uint32_t nSubnets = 5;
    uint32_t nHosts = 10;

    NodeContainer allNodes;
    InternetStackHelper stack;
    Ipv4AddressHelper address;

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        NodeContainer subnetNodes;
        subnetNodes.Create (nHosts);
        allNodes.Add (subnetNodes);
        stack.Install (subnetNodes);

        // Position nodes (each subnet in one row)
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

        for (uint32_t j = 0; j < nHosts; ++j) {
            positionAlloc->Add (Vector (j * 10.0, i * 20.0, 0.0));
        }

        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (subnetNodes);

        // Connect all nodes in subnet (LAN)
        NetDeviceContainer devices = csma.Install (subnetNodes);

        // Assign subnet IP safely
        std::stringstream ss;
        ss << "192.168." << i << ".0";
        std::string baseIp = ss.str(); // Prevents temporary string destruction issues
        address.SetBase (baseIp.c_str(), "255.255.255.0");

        Ipv4InterfaceContainer interfaces = address.Assign (devices);

        // Print all host IPs
        std::cout << "\n=== Subnet " << i << " ===" << std::endl;
        for (uint32_t j = 0; j < nHosts; ++j) {
            std::cout << "Host-" << j << " : "
                      << interfaces.GetAddress(j) << std::endl;
        }
    }

    // NetAnim configuration
    AnimationInterface anim ("subnet_simulation.xml");

    // Show IP on each node
    for (uint32_t i = 0; i < allNodes.GetN (); ++i) {
        Ptr<Ipv4> ipv4 = allNodes.Get(i)->GetObject<Ipv4>();

        std::ostringstream ip;
        // Index 1 is the CSMA interface (Index 0 is loopback)
        ip << ipv4->GetAddress(1,0).GetLocal();

        // Safely use GetId() for the node description update
        anim.UpdateNodeDescription(
            allNodes.Get(i)->GetId(),
            "Host-" + std::to_string(i) + "\n" + ip.str()
        );
    }

    // Stop the simulation at 10 seconds so NetAnim captures the layout properly
    Simulator::Stop (Seconds (10.0)); 
    Simulator::Run ();
    Simulator::Destroy ();
    
    return 0;
}
