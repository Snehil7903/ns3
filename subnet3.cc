#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

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

        // 🔹 Position nodes (each subnet in one row)
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

        for (uint32_t j = 0; j < nHosts; ++j) {
            positionAlloc->Add (Vector (j * 10.0, i * 20.0, 0.0));
        }

        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (subnetNodes);

        // 🔹 Connect all nodes in subnet (LAN)
        NetDeviceContainer devices = csma.Install (subnetNodes);

        // 🔹 Assign subnet IP
        std::stringstream ss;
        ss << "192.168." << i << ".0";
        address.SetBase (ss.str ().c_str (), "255.255.255.0");

        Ipv4InterfaceContainer interfaces = address.Assign (devices);

        // 🔥 Print all host IPs
        std::cout << "\n=== Subnet " << i << " ===" << std::endl;
        for (uint32_t j = 0; j < nHosts; ++j) {
            std::cout << "Host-" << j << " : "
                      << interfaces.GetAddress(j) << std::endl;
        }
    }

    // 🔹 NetAnim
    AnimationInterface anim ("subnet_simulation.xml");

    // 🔥 Show IP on each node
    for (uint32_t i = 0; i < allNodes.GetN (); ++i) {
        Ptr<Ipv4> ipv4 = allNodes.Get(i)->GetObject<Ipv4>();

        std::ostringstream ip;
        ip << ipv4->GetAddress(1,0).GetLocal();

        anim.UpdateNodeDescription(
            allNodes.Get(i),
            "Host-" + std::to_string(i) + "\n" + ip.str()
        );
    }

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}