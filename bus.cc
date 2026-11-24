#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BusTopologyWithNetAnim");

int main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create 4 nodes for the CSMA bus topology
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Configure CSMA channel attributes
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer devices = csma.Install(nodes);

    // 3. Install Internet stack and assign IPv4 addresses
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 4. Setup UDP Echo Server on Node 0
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 5. Setup UDP Echo Clients on Nodes 1, 2, and 3
    const uint32_t totalNodes = nodes.GetN();
    for (uint32_t i = 1; i < totalNodes; ++i)
    {
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(3));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + static_cast<double>(i)));
        clientApp.Stop(Seconds(10.0));
    }

    // 6. Configure Mobility Model with explicit coordinates
    double xStart = 10.0;
    double yPos = 30.0;
    double nodeSpacing = 20.0;

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < totalNodes; ++i)
    {
        double xPos = xStart + (static_cast<double>(i) * nodeSpacing);
        positionAlloc->Add(Vector(xPos, yPos, 0.0));
    }

    MobilityHelper mobility;
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // 7. Configure NetAnim
    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true);

    for (uint32_t i = 0; i < totalNodes; ++i)
    {
        Ptr<Node> node = nodes.Get(i);

        if (i == 0)
        {
            anim.UpdateNodeDescription(node, "Server");
            anim.UpdateNodeColor(node, 255, 0, 0); // Red for Server
        }
        else
        {
            anim.UpdateNodeDescription(node, "Client " + std::to_string(i));
            anim.UpdateNodeColor(node, 0, 0, 255); // Blue for Clients
        }
    }

    // Enable PCAP tracing for Wireshark analysis
    csma.EnablePcapAll("bus-topology", false);

    // 8. Run simulation
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
