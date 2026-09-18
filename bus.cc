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
    CommandLine cmd;
    cmd.Parse(argc, argv);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(4);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    auto devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    auto interfaces = address.Assign(devices);

    // Modern ns-3 apps can use smart pointers or cleanly grouped application helpers
    UdpEchoServerHelper echoServer(9);
    auto serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Modern C++: Used std::size_t over legacy raw uint32_t for container sizing
    for (std::size_t i = 1, total_nodes = nodes.GetN(); i < total_nodes; ++i) 
    {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        auto clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + static_cast<double>(i)));
        clientApp.Stop(Seconds(10.0));
    }

    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true);

    constexpr double x_start = 10.0;
    constexpr double y_pos = 30.0;
    constexpr double node_spacing = 20.0;

    for (std::size_t i = 0, total_nodes = nodes.GetN(); i < total_nodes; ++i) 
    {
        auto node = nodes.Get(i);
        
        anim.SetConstantPosition(node, x_start + static_cast<double>(i) * node_spacing, y_pos);
        
        // Modern C++: Optimized string building using cleaner ternary conditions
        std::string desc = (i == 0) ? "Server" : "Client " + std::to_string(i);
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
