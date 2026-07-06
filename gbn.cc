#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-apps-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
    uint32_t nSubnets = 5;
    uint32_t nHosts = 10;

    // 1. Create Nodes
    NodeContainer router;
    router.Create(1);

    // 2. Setup Router Mobility
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> routerPos = CreateObject<ListPositionAllocator>();
    routerPos->Add(Vector(50.0, 75.0, 0.0)); 
    mobility.SetPositionAllocator(routerPos);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(router);

    InternetStackHelper stack;
    stack.Install(router);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    Ipv4AddressHelper address;
    Ipv4Mask mask("255.255.255.240");

    std::vector<NodeContainer> subnetHosts(nSubnets);
    std::vector<Ipv4InterfaceContainer> interfaces(nSubnets);

    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        subnetHosts[i].Create(nHosts);
        stack.Install(subnetHosts[i]);

        NodeContainer network;
        network.Add(router.Get(0));
        network.Add(subnetHosts[i]);

        csma.Install(network);

        std::stringstream ss;
        ss << "192.168.72." << (i * 16);
        address.SetBase(ss.str().c_str(), mask);
        interfaces[i] = address.Assign(csma.Install(network)); 

        // Positioning for Hosts
        Ptr<ListPositionAllocator> hostPos = CreateObject<ListPositionAllocator>();
        for (uint32_t j = 0; j < nHosts; ++j)
        {
            hostPos->Add(Vector(150.0 + (j * 20.0), i * 30.0, 0.0));
        }
        mobility.SetPositionAllocator(hostPos);
        mobility.Install(subnetHosts[i]);
    }

    // FIX: Enable Forwarding on the router globally
    Ptr<Ipv4> ipv4 = router.Get(0)->GetObject<Ipv4>();
    ipv4->SetAttribute("IpForward", BooleanValue(true));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ---- Ping Application ----
    Ipv4Address targetIp = interfaces[4].GetAddress(1); 
    PingHelper ping(targetIp);
    ping.SetAttribute("Verbose", BooleanValue(true));

    ApplicationContainer app = ping.Install(subnetHosts[0].Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // ---- NetAnim ----
    AnimationInterface anim("five_subnets.xml");
    anim.UpdateNodeDescription(router.Get(0), "MainRouter");
    anim.UpdateNodeColor(router.Get(0), 255, 0, 0);

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
