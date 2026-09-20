#include <format>
#include <string>
#include <string_view>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BusTopologyWithNetAnim");

int main (int argc, char *argv[]) 
{
    // Modern ns-3: Pass __FILE__ to CommandLine for better usage reporting
    CommandLine cmd (__FILE__);
    cmd.Parse(argc, argv);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(4);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    // Idiomatic ns-3: Use string representation for time attributes
    csma.SetChannelAttribute("Delay", StringValue("6560ns"));

    auto devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    auto interfaces = address.Assign(devices);

    // Setup UDP Echo Server on Node 0
    UdpEchoServerHelper echoServer(9);
    auto serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Modern ns-3 API alignment: Use uint32_t to match GetN() and Get() signatures
    const uint32_t totalNodes = nodes.GetN();
    for (uint32_t i = 1; i < totalNodes; ++i) 
    {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        auto clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + static_cast<double>(i)));
        clientApp.Stop(Seconds(10.0));
    }

    // NetAnim Configuration
    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true);

    constexpr double xStart = 10.0;
    constexpr double yPos = 30.0;
    constexpr double nodeSpacing = 20.0;

    for (uint32_t i = 0; i < totalNodes; ++i) 
    {
        auto node = nodes.Get(i);
        
        anim.SetConstantPosition(node, xStart + static_cast<double>(i) * nodeSpacing, yPos);
        
        // Modern C++20: Replaced legacy std::to_string with efficient std::format
        std::string desc = (i == 0) ? "Server" : std::format("Client {}", i);
        anim.UpdateNodeDescription(node, desc);

        if (i == 0) 
        {
            anim.UpdateNodeColor(node, 255, 0, 0); // Red for Server
        } 
        else 
        {
            anim.UpdateNodeColor(node, 0, 0, 255); // Blue for Clients
        }
    }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
