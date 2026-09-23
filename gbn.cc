#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <ranges>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
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

    // 1. Initialize Network Architecture Elements
    NodeContainer routerNode;
    routerNode.Create(1);

    InternetStackHelper internetStack;
    internetStack.Install(routerNode);

    // Explicitly enable IPv4 Routing/Forwarding on the Router node
    auto ipv4Router = routerNode.Get(0)->GetObject<Ipv4>();
    ipv4Router->SetAttribute("IpForward", BooleanValue(true));

    // 2. Configure Global Mobility
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    auto routerPosition = CreateObject<ListPositionAllocator>();
    routerPosition->Add(Vector(50.0, 75.0, 0.0)); 
    mobility.SetPositionAllocator(routerPosition);
    mobility.Install(routerNode);

    // 3. Configure Network Channels & Addressing
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    Ipv4AddressHelper addressAllocator;
    const Ipv4Mask subnetMask{"255.255.255.240"}; // /28 subnet configuration

    std::vector<NodeContainer> subnetHosts(NUM_SUBNETS);
    std::vector<Ipv4InterfaceContainer> interfaces(NUM_SUBNETS);

    // 4. Construct Subnets dynamically using Modern C++ Ranges
    for (const auto subnetIdx : std::views::iota(0u, NUM_SUBNETS))
    {
        subnetHosts[subnetIdx].Create(NUM_HOSTS);
        internetStack.Install(subnetHosts[subnetIdx]);

        // Construct Local Shared Network Segment (Bus Topology)
        NodeContainer localNetwork;
        localNetwork.Add(routerNode.Get(0));
        localNetwork.Add(subnetHosts[subnetIdx]);

        auto netDevices = csma.Install(localNetwork);

        // Modern compile-time safe string interpolation for IP creation
        std::string baseIp = std::format("192.168.72.{}", subnetIdx * 16);
        addressAllocator.SetBase(baseIp.c_str(), subnetMask);
        interfaces[subnetIdx] = addressAllocator.Assign(netDevices); 

        // Set Host Topology Grid Coordinates
        auto hostPositions = CreateObject<ListPositionAllocator>();
        for (const auto hostIdx : std::views::iota(0u, NUM_HOSTS))
        {
            hostPositions->Add(Vector(150.0 + (hostIdx * 20.0), (subnetIdx + 1) * 40.0, 0.0));
        }
        mobility.SetPositionAllocator(hostPositions);
        mobility.Install(subnetHosts[subnetIdx]);
    }

    // 5. Establish Global Network Routes
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Deploy Simulation Applications (Ping Subnet 0 Host 0 -> Subnet 4 Host 0)
    Ipv4Address destinationIp = interfaces[4].GetAddress(1); 
    
    PingHelper pingApp(destinationIp);
    pingApp.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));

    auto appContainer = pingApp.Install(subnetHosts[0].Get(0));
    appContainer.Start(Seconds(1.0));
    appContainer.Stop(Seconds(10.0));

    // 7. Initialize NetAnim Visualizer
    AnimationInterface visualizer{"five_subnets.xml"};
    visualizer.UpdateNodeDescription(routerNode.Get(0), "MainRouter");
    visualizer.UpdateNodeColor(routerNode.Get(0), 255, 0, 0);

    for (const auto subnetIdx : std::views::iota(0u, NUM_SUBNETS))
    {
        for (const auto hostIdx : std::views::iota(0u, NUM_HOSTS))
        {
            uint8_t blueGradient = static_cast<uint8_t>(255 - (subnetIdx * 40));
            auto currentHost = subnetHosts[subnetIdx].Get(hostIdx);
            
            visualizer.UpdateNodeDescription(currentHost, "Host");
            visualizer.UpdateNodeColor(currentHost, 0, 0, blueGradient);
        }
    }

    // 8. Run Network Engine Environment
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
