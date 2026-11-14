#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include <cmath>
#include <vector>
#include <string>

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

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    std::vector<Ipv4InterfaceContainer> linkInterfaces;

    // Connect each leaf to the hub
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        NodeContainer pair(hubNode.Get(0), leafNodes.Get(i));
        NetDeviceContainer linkDevices = p2p.Install(pair);

        Ipv4InterfaceContainer iface = address.Assign(linkDevices);
        linkInterfaces.push_back(iface);

        // Automatically increment to the next /24 subnet (e.g., 10.1.2.0, 10.1.3.0...)
        address.NewNetwork();
    }

    // Dynamic simulation duration so all staggered clients finish
    double simStopTime = 5.0 + nLeaf;

    // 4. Install UDP Echo Server on Hub
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(hubNode.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(simStopTime));

    // 5. Install UDP Echo Clients on Leaves
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        Ipv4Address hubAddress = linkInterfaces[i].GetAddress(0);

        UdpEchoClientHelper echoClient(hubAddress, 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(leafNodes.Get(i));
        clientApp.Start(Seconds(2.0 + i));
        clientApp.Stop(Seconds(simStopTime));
    }

    // Initialize Global Routing Tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Mobility Setup
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(hubNode);
    mobility.Install(leafNodes);

    // 7. NetAnim Visualization Setup
    AnimationInterface anim("star-topology.xml");

    Ptr<Node> hubPtr = hubNode.Get(0);
    anim.SetConstantPosition(hubPtr, 50.0, 50.0);
    anim.UpdateNodeDescription(hubPtr, "Hub");
    anim.UpdateNodeColor(hubPtr, 255, 0, 0); // Red

    double radius = 30.0;
    for (uint32_t i = 0; i < nLeaf; i++)
    {
        Ptr<Node> leafPtr = leafNodes.Get(i);

        double angle = (360.0 / nLeaf) * i;
        double rad = angle * M_PI / 180.0;

        double x = 50.0 + radius * std::cos(rad);
        double y = 50.0 + radius * std::sin(rad);

        anim.SetConstantPosition(leafPtr, x, y);
        anim.UpdateNodeDescription(leafPtr, "Leaf-" + std::to_string(i));
        anim.UpdateNodeColor(leafPtr, 0, 0, 255); // Blue
    }

    anim.EnablePacketMetadata(true);

    // 8. Run Simulation
    Simulator::Stop(Seconds(simStopTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
