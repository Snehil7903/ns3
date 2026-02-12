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
    // Use CommandLine to allow changing parameters at runtime
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    // Enable logging to see the communication in the terminal
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create 4 nodes on the bus
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Configure CSMA (Models a shared bus cable)
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // 3. Install Devices on all nodes
    NetDeviceContainer devices = csma.Install(nodes);

    // 4. Install Internet Stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // 5. Assign IP Addresses (Single subnet for the entire bus)
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 6. Install UDP Echo Server on Node 0
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 7. Install UDP Echo Clients on Nodes 1, 2, and 3
    for (uint32_t i = 1; i < nodes.GetN(); i++) 
    {
        // All clients target the IP of Node 0
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + i)); // Staggered start to prevent collisions
        clientApp.Stop(Seconds(10.0));
    }

    // 8. NetAnim Visualization Section
    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true); // Allows viewing packet info in NetAnim

    // Arrange nodes in a horizontal bus layout
    int x_start = 10;
    int y_pos = 30;
    for (uint32_t i = 0; i < nodes.GetN(); i++) 
    {
        anim.SetConstantPosition(nodes.Get(i), x_start + i * 20, y_pos);
        
        std::string desc = (i == 0) ? "Server" : "Client " + std::to_string(i);
        anim.UpdateNodeDescription(nodes.Get(i), desc);

        // Server is Red, Clients are Blue
        if (i == 0)
            anim.UpdateNodeColor(nodes.Get(i), 255, 0, 0);
        else
            anim.UpdateNodeColor(nodes.Get(i), 0, 0, 255);
    }

    // 9. Simulation Execution
    Simulator::Stop(Seconds(11.0)); // Ensure simulation stops after apps
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}