#include <iostream>
#include <vector>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/v4ping-helper.h" 

using namespace ns3;

int main (int argc, char *argv[])
{
    // Configuration constants
    const uint32_t nSubnets = 5;
    const uint32_t nHosts = 10;

    // 1. Create Router
    NodeContainer router;
    router.Create(1);

    // 2. Setup Mobility for Router (Center point)
    MobilityHelper mobility;
    auto routerPos = CreateObject<ListPositionAllocator>();
    routerPos->Add(Vector(50.0, 75.0, 0.0)); 
    mobility.SetPositionAllocator(routerPos);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(router);

    // Install Internet Stack on Router
    InternetStackHelper stack;
    stack.Install(router);

    // Explicitly enable IP forwarding on the router so it can route between subnets
    auto routerIpv4 = router.Get(0)->GetObject<Ipv4>();
    routerIpv4->SetAttribute("IpForward", BooleanValue(true));

    Ipv4AddressHelper address;
    const Ipv4Mask mask("255.255.255.240"); // Provides 14 usable IPs per subnet (.1 to .14)

    std::vector<NodeContainer> subnetHosts(nSubnets);
    std::vector<Ipv4InterfaceContainer> interfaces(nSubnets);

    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        // Define a distinct CSMA helper per loop to create independent collision domains
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        // Create hosts for this subnet
        subnetHosts[i].Create(nHosts);
        stack.Install(subnetHosts[i]);

        // Separate container for devices on this specific subnet bus
        NetDeviceContainer meshDevices;
        
        // Install CSMA net device on the router and the subnet hosts for this channel specifically
        meshDevices.Add(csma.Install(router.Get(0)));
        meshDevices.Add(csma.Install(subnetHosts[i]));

        // Assign IP Addresses ensuring no overlap by striding by 16 using C++ string creation
        std::string subnetIp = "192.168.72." + std::to_string(i * 16);
        address.SetBase(Ipv4Address(subnetIp.c_str()), mask);
        interfaces[i] = address.Assign(meshDevices);

        // Positioning for Hosts in NetAnim (Vertical rows)
        auto hostPos = CreateObject<ListPositionAllocator>();
        for (uint32_t j = 0; j < nHosts; ++j)
        {
            hostPos->Add(Vector(150.0 + (j * 20.0), (i + 1) * 30.0, 0.0));
        }
        mobility.SetPositionAllocator(hostPos);
        mobility.Install(subnetHosts[i]);
    }

    // Populate routing tables after all networks are created and IPs are assigned
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ---- Ping Application ----
    // Target: Subnet 5 (index 4), Host 0. 
    // Index 0 of interface is the Router, so index 1 is the first host.
    Ipv4Address targetIp = interfaces[4].GetAddress(1); 
    
    V4PingHelper ping(targetIp);
    ping.SetAttribute("Verbose", BooleanValue(true));

    // Install on Subnet 1 (index 0), Host 0
    ApplicationContainer app = ping.Install(subnetHosts[0].Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // ---- NetAnim ----
    AnimationInterface anim("five_subnets.xml");
    anim.UpdateNodeDescription(router.Get(0), "MainRouter");
    anim.UpdateNodeColor(router.Get(0), 255, 0, 0); // Red router

    // Set duration and run the simulation
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
