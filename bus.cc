#include <string>
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

    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    for (uint32_t i = 1; i < nodes.GetN(); i++) 
    {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + i));
        clientApp.Stop(Seconds(10.0));
    }

    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true);

    int x_start = 10;
    int y_pos = 30;
    for (uint32_t i = 0; i < nodes.GetN(); i++) 
    {
        anim.SetConstantPosition(nodes.Get(i), x_start + i * 20, y_pos);
        
        std::string desc = (i == 0) ? "Server" : "Client " + std::to_string(i);
        anim.UpdateNodeDescription(nodes.Get(i), desc);

        if (i == 0)
            anim.UpdateNodeColor(nodes.Get(i), 255, 0, 0);
        else
            anim.UpdateNodeColor(nodes.Get(i), 0, 0, 255);
    }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
