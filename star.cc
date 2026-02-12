#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include <cmath>
#include <vector>

// Ensure M_PI is defined for math calculations
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StarTopologyWithNetAnim");

int main(int argc, char *argv[])
{
    uint32_t nLeaf = 4;

    CommandLine cmd;
    cmd.AddValue("nLeaf", "Number of leaf nodes", nLeaf);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create Nodes
    NodeContainer hubNode;
    hubNode.Create(1);

    NodeContainer leafNodes;
    leafNodes.Create(nLeaf);

    // 2. Install Internet Stack
    InternetStackHelper stack;
    stack.Install(hubNode);
    stack.Install(leafNodes);

    // 3. Point-to-Point Configuration
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4InterfaceContainer interfaces;

    // Connect each leaf to the hub
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        NodeContainer pair(hubNode.Get(0), leafNodes.Get(i));
        NetDeviceContainer linkDevices = p2p.Install(pair);

        // Assign a unique subnet to each point-to-point link
        std::ostringstream subnet;
        subnet << "10.1." << i + 1 << ".0";

        Ipv4AddressHelper address;
        address.SetBase(subnet.str().c_str(), "255.255.255.0");

        Ipv4InterfaceContainer iface = address.Assign(linkDevices);
        interfaces.Add(iface); 
    }

    // 4. Install UDP Echo Server on Hub
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(hubNode.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));

    // 5. Install UDP Echo Clients on Leaves
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        // The Hub's IP for this specific link is the first index (0) of each 2-node interface pair
        Ipv4Address hubAddress = interfaces.GetAddress(i * 2, 0);

        UdpEchoClientHelper echoClient(hubAddress, 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(leafNodes.Get(i));
        clientApp.Start(Seconds(2.0 + i)); // Staggered start times
        clientApp.Stop(Seconds(20.0));
    }

    // 6. NetAnim Visualization Setup
    AnimationInterface anim("star-topology.xml");

    // Position Hub at (50, 50)
    anim.SetConstantPosition(hubNode.Get(0), 50.0, 50.0);
    anim.UpdateNodeDescription(hubNode.Get(0), "Hub");
    anim.UpdateNodeColor(hubNode.Get(0), 255, 0, 0); // Red

    // Position Leaves in a circle around the Hub
    double radius = 30.0;
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        double angle = (360.0 / nLeaf) * i;
        double rad = angle * M_PI / 180.0;

        double x = 50.0 + radius * std::cos(rad);
        double y = 50.0 + radius * std::sin(rad);

        anim.SetConstantPosition(leafNodes.Get(i), x, y);
        anim.UpdateNodeDescription(leafNodes.Get(i), "Leaf-" + std::to_string(i));
        anim.UpdateNodeColor(leafNodes.Get(i), 0, 0, 255); // Blue
    }

    // Optional: Enable packet tracing to see packet flow in NetAnim
    anim.EnablePacketMetadata(true); 

    // 7. Run Simulation
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}