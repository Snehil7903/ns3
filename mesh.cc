#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FullMeshTopology");

int main(int argc, char *argv[])
{
    uint32_t nNodes = 5; // Total nodes in the mesh

    CommandLine cmd;
    cmd.AddValue("nNodes", "Number of nodes in the mesh", nNodes);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    // 1. Create Nodes
    NodeContainer nodes;
    nodes.Create(nNodes);

    // 2. Install Internet Stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // 3. Configure Point-to-Point Links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4AddressHelper address;
    uint32_t subnet = 1;

    // 4. Create Full Mesh (Connect every node to every other node)
    for (uint32_t i = 0; i < nNodes; i++)
    {
        for (uint32_t j = i + 1; j < nNodes; j++)
        {
            NodeContainer pair(nodes.Get(i), nodes.Get(j));
            NetDeviceContainer devices = p2p.Install(pair);

            // Assign a unique subnet for every single link in the mesh
            std::ostringstream oss;
            oss << "10.1." << subnet++ << ".0";
            address.SetBase(oss.str().c_str(), "255.255.255.0");
            address.Assign(devices);
        }
    }

    // 5. Global Routing (Crucial for Mesh)
    // This tells nodes how to jump through neighbors to reach distant nodes
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Install Application (Example: Node 0 sends to Node 4)
    uint32_t destNode = nNodes - 1;
    // We get the IP address of the destination node (Node 4) 
    // from the first interface it has.
    Ptr<Ipv4> ipv4 = nodes.Get(destNode)->GetObject<Ipv4>();
    Ipv4Address addr = ipv4->GetAddress(1, 0).GetLocal(); 

    UdpEchoClientHelper echoClient(addr, 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = echoClient.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(destNode));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 7. NetAnim Visualization
    AnimationInterface anim("mesh-topology.xml");
    
    double radius = 30.0;
    double centerX = 50.0, centerY = 50.0;

    for (uint32_t i = 0; i < nNodes; i++)
    {
        double angle = (360.0 / nNodes) * i;
        double rad = angle * M_PI / 180.0;
        double x = centerX + radius * std::cos(rad);
        double y = centerY + radius * std::sin(rad);

        anim.SetConstantPosition(nodes.Get(i), x, y);
        anim.UpdateNodeDescription(nodes.Get(i), "Node " + std::to_string(i));
    }

    anim.EnablePacketMetadata(true);

    // 8. Run Simulation
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}