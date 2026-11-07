#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <ranges>
#include <algorithm>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h" 
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

// Configuration Constants
constexpr uint32_t NUM_SUBNETS = 5;
constexpr uint32_t NUM_HOSTS = 10;

int main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Enable IP Forwarding globally so the central router can route between subnets
    Config::SetDefault("ns3::Ipv4::IpForward", BooleanValue(true));

    // 1. Initialize Core Router Architecture
    NodeContainer routerNode;
    routerNode.Create(1);

    InternetStackHelper internetStack;
    internetStack.Install(routerNode);

    // 2. Configure Global Mobility
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    auto routerPosition = CreateObject<ListPositionAllocator>();
    routerPosition->Add(Vector(50.0, 75.0, 0.0)); 
    mobility.SetPositionAllocator(routerPosition);
    mobility.Install(routerNode);

    // 3. Configure Physical Layer Media
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    Ipv4AddressHelper addressAllocator;
    const Ipv4Mask subnetMask{"255.255.255.240"}; // /28 subnet (14 usable hosts)

    std::vector<NodeContainer> subnetHosts(NUM_SUBNETS);
    std::vector<Ipv4InterfaceContainer> interfaces(NUM_SUBNETS);
    Ipv4StaticRoutingHelper staticRoutingHelper;

    // 4. Construct Topologies Dynamically via Modern C++ Ranges
    for (const auto subnetIdx : std::views::iota(0u, NUM_SUBNETS))
    {
        auto& currentSubnet = subnetHosts[subnetIdx];
        currentSubnet.Create(NUM_HOSTS);
        internetStack.Install(currentSubnet);

        // Build localized bus topology segment (Link containing Router + local hosts)
        NodeContainer localNetwork;
        localNetwork.Add(routerNode.Get(0));
        localNetwork.Add(currentSubnet);

        auto netDevices = csma.Install(localNetwork);

        // Assign IP addresses (Index 0 = Router, Indices 1..10 = Hosts)
        std::string baseIp = std::format("192.168.72.{}", subnetIdx * 16);
        addressAllocator.SetBase(baseIp.c_str(), subnetMask);
        interfaces[subnetIdx] = addressAllocator.Assign(netDevices); 

        // FIX: Set Default Gateway on each host pointing to the Router's subnet IP
        Ipv4Address routerGateway = interfaces[subnetIdx].GetAddress(0);
        for (const auto hostIdx : std::views::iota(0u, NUM_HOSTS))
        {
            Ptr<Ipv4> hostIpv4 = currentSubnet.Get(hostIdx)->GetObject<Ipv4>();
            Ptr<Ipv4StaticRouting> hostStaticRouting = staticRoutingHelper.GetStaticRouting(hostIpv4);
            // Interface 1 is the CSMA device (Interface 0 is loopback)
            hostStaticRouting->SetDefaultRoute(routerGateway, 1);
        }

        // Set Host Topology Grid Coordinates
        auto hostPositions = CreateObject<ListPositionAllocator>();
        for (const auto hostIdx : std::views::iota(0u, NUM_HOSTS))
        {
            hostPositions->Add(Vector(150.0 + (hostIdx * 20.0), (subnetIdx + 1) * 40.0, 0.0));
        }
        mobility.SetPositionAllocator(hostPositions);
        mobility.Install(currentSubnet);
    }

    // 5. Compute Autonomous Global Routing Matrices (for the router)
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Deploy Network Testing Applications (Ping Subnet 0 Host 0 -> Subnet 4 Host 0)
    const Ipv4Address destinationIp = interfaces[4].GetAddress(1); 
    
    PingHelper pingApp(destinationIp);
    // FIX: Use VerboseMode instead of Verbose in modern ns-3
    pingApp.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));

    auto appContainer = pingApp.Install(subnetHosts[0].Get(0));
    appContainer.Start(Seconds(1.0));
    appContainer.Stop(Seconds(10.0));

    // 7. Initialize NetAnim Visualizer Configuration
    AnimationInterface visualizer{"five_subnets.xml"};
    visualizer.UpdateNodeDescription(routerNode.Get(0), "MainRouter");
    visualizer.UpdateNodeColor(routerNode.Get(0), 255, 0, 0);

    for (const auto subnetIdx : std::views::iota(0u, NUM_SUBNETS))
    {
        const uint8_t blueGradient = static_cast<uint8_t>(255 - (subnetIdx * 40));
        
        for (const auto hostIdx : std::views::iota(0u, NUM_HOSTS))
        {
            auto currentHost = subnetHosts[subnetIdx].Get(hostIdx);
            visualizer.UpdateNodeDescription(currentHost, "Host");
            visualizer.UpdateNodeColor(currentHost, 0, 0, blueGradient);
        }
    }

    // 8. Execute Discrete Event Simulation Engine
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
