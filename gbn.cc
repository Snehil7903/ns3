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

using namespace ns3;

int main(int argc, char *argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    const uint32_t nSubnets = 5;
    const uint32_t nHosts = 10;

    // 1. Create Router Node
    NodeContainer router;
    router.Create(1);

    // 2. Setup Router Mobility
    MobilityHelper mobility;
    auto routerPos = CreateObject<ListPositionAllocator>();
    routerPos->Add(Vector(50.0, 75.0, 0.0)); 
    mobility.SetPositionAllocator(routerPos);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(router);

    // 3. Install Internet Stack on Router
    InternetStackHelper stack;
    stack.Install(router);

    // Explicitly enable IPv4 Routing/Forwarding on the Router node
    Ptr<Ipv4> ipv4Router = router.Get(0)->GetObject<Ipv4>();
    ipv4Router->SetAttribute("IpForward", BooleanValue(true));

    // 4. Setup CSMA Channel Configuration
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    Ipv4AddressHelper address;
    Ipv4Mask mask("255.255.255.240"); // /28 subnet mask allows up to 14 hosts per subnet

    std::vector<NodeContainer> subnetHosts(nSubnets);
    std::vector<Ipv4InterfaceContainer> interfaces(nSubnets);

    // 5. Populate Subnets
    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        subnetHosts[i].Create(nHosts);
        stack.Install(subnetHosts[i]);

        // Topology: Link the router interface and all subnet hosts together on one CSMA bus
        NodeContainer network;
        network.Add(router.Get(0));
        network.Add(subnetHosts[i]);

        // Install CSMA devices
        NetDeviceContainer devices = csma.Install(network);

        // Generate base subnet IPs safely using standard string concatenation
        std::string subnetStr = "192.168.72." + std::to_string(i * 16);
        
        address.SetBase(subnetStr.c_str(), mask);
        interfaces[i] = address.Assign(devices); 

        // Positioning for Hosts
        auto hostPos = CreateObject<ListPositionAllocator>();
        for (uint32_t j = 0; j < nHosts; ++j)
        {
            hostPos->Add(Vector(150.0 + (j * 20.0), (i + 1) * 40.0, 0.0));
        }
        mobility.SetPositionAllocator(hostPos);
        mobility.Install(subnetHosts[i]);
    }

    // 6. Build global routing paths across all subnets
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 7. Setup Ping Application (Host 0 in Subnet 0 pings Host 0 in Subnet 4)
    // index 0 of interface is the router interface on that subnet; index 1 is host 0.
    Ipv4Address targetIp = interfaces[4].GetAddress(1); 
    
    PingHelper ping(targetIp);
    // Modern ns-3 uses VerboseMode Enum rather than a plain boolean for the Ping application
    ping.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));

    ApplicationContainer app = ping.Install(subnetHosts[0].Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // 8. NetAnim Visualizer
    AnimationInterface anim("five_subnets.xml");
    anim.UpdateNodeDescription(router.Get(0), "MainRouter");
    anim.UpdateNodeColor(router.Get(0), 255, 0, 0);

    // Set colors for the hosts to easily distinguish subnets visually
    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        for (uint32_t j = 0; j < nHosts; ++j)
        {
            anim.UpdateNodeDescription(subnetHosts[i].Get(j), "Host");
            anim.UpdateNodeColor(subnetHosts[i].Get(j), 0, 0, 255 - (i * 40));
        }
    }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
