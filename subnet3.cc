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

int main(int argc, char *argv[]) 
{
    // Modern CommandLine initialization without outdated macro parameters
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging to observe packet processing in the console
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create a container for the 4 nodes on the shared bus
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Configure CSMA parameters (Shared Bus Cable)
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // 3. Install network interfaces on all allocated nodes
    NetDeviceContainer devices = csma.Install(nodes);

    // 4. Install the network stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // 5. Assign IPv4 addresses within a single subnet mask
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 6. Install UDP Echo Server on Node 0
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 7. Install UDP Echo Clients on remaining nodes using modern type definitions
    for (std::size_t i = 1; i < nodes.GetN(); ++i) 
    {
        // All clients target the IP assigned to Node 0
        UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0 + i)); // Stagger start time to prevent collisions
        clientApp.Stop(Seconds(10.0));
    }

    // Initialize layout tracking via a stationary mobility model
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // 8. NetAnim Visualization Configuration
    AnimationInterface anim("bus-topology.xml");
    anim.EnablePacketMetadata(true); 

    // Compute standard horizontal offsets for nodes in the layout
    const double xStart = 10.0;
    const double yPos = 30.0;
    
    for (std::size_t i = 0; i < nodes.GetN(); ++i) 
    {
        anim.SetConstantPosition(nodes.Get(i), xStart + (i * 20.0), yPos);
        
        std::string desc = (i == 0) ? "Server" : "Client " + std::to_string(i);
        anim.UpdateNodeDescription(nodes.Get(i), desc); 

        // Apply strict color scheme: Server is Red, Clients are Blue
        if (i == 0)
        {
            anim.UpdateNodeColor(nodes.Get(i), 255, 0, 0); 
        }
        else
        {
            anim.UpdateNodeColor(nodes.Get(i), 0, 0, 255); 
        }
    }

    // 9. Simulation Processing
    Simulator::Stop(Seconds(11.0)); 
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
