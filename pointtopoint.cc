#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int main(int argc, char *argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Time::SetResolution(Time::NS) is omitted as it is the default 
    // and can cause static initialization crashes in modern ns-3.

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create Nodes
    NodeContainer nodes;
    nodes.Create(2);

    // 2. Configure Point-to-Point Link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices = pointToPoint.Install(nodes);

    // 3. Install Internet Stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // 4. Assign IP Addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 5. Install UDP Echo Server (Node 1)
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // 6. Install UDP Echo Client (Node 0)
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // 7. NetAnim Visualization Configuration
    AnimationInterface anim("first.xml");

    anim.SetConstantPosition(nodes.Get(0), 10.0, 20.0);
    anim.SetConstantPosition(nodes.Get(1), 30.0, 20.0);

    anim.UpdateNodeDescription(nodes.Get(0), "Client");
    anim.UpdateNodeDescription(nodes.Get(1), "Server");

    anim.UpdateNodeColor(nodes.Get(0), 0, 0, 255); // Blue Client
    anim.UpdateNodeColor(nodes.Get(1), 255, 0, 0); // Red Server
    
    anim.EnablePacketMetadata(true); // Added so you can trace the packets in NetAnim

    // 8. Run Simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
