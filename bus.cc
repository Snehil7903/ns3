#include <string>
#include <sstream>
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
    // Enable usage reporting safely
    CommandLine cmd (__FILE__);
    cmd.Parse(argc, argv);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(4);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    // FIX: Set Delay using TimeValue instead of StringValue
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560))); 

    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Setup UDP Echo Server on Node 0
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Setup UDP Echo Clients on Nodes 1, 2, and 3
    const uint32_t totalNodes = nodes.GetN();
    for (uint32_t i = 1; i < totalNodes; ++i) 
    {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + static_cast<double>(i)));
        clientApp.Stop(Seconds(10.0));
    }

    // NetAnim Configuration - Initialize BEFORE configuring node elements
    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true);

    constexpr double xStart = 10.0;
    constexpr double yPos = 30.0;
    constexpr double nodeSpacing = 20.0;

    for (uint32_t i = 0; i < totalNodes; ++i) 
    {
        Ptr<Node> node = nodes.Get(i);
        
        // FIX: Call SetConstantPosition using the correct static scope
        AnimationInterface::SetConstantPosition(node, xStart + static_cast<double>(i) * nodeSpacing, yPos);
        
        // Use robust std::ostringstream to bypass compiler-specific standard levels
        std::ostringstream oss;
        if (i == 0) 
        {
            oss << "Server";
            anim.UpdateNodeDescription(node, oss.str());
            anim.UpdateNodeColor(node, 255, 0, 0);       // Red for Server
        } 
        else 
        {
            oss << "Client " << i;
            anim.UpdateNodeDescription(node, oss.str());
            anim.UpdateNodeColor(node, 0, 0, 255);       // Blue for Clients
        }
    }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
