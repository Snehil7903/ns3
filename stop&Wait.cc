#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
// Use the precise header to guarantee modern ns-3 compatibility
#include "ns3/netanim-module.h" 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StopAndWaitARQ");

int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging to see the "Send" and "Receive" events clearly in the terminal
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1. Create 2 Nodes (Sender and Receiver)
    NodeContainer nodes;
    nodes.Create(2);

    // 2. Configure Point-to-Point Link
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("512Kbps"));
    p2p.SetChannelAttribute("Delay", StringValue("50ms")); // 50ms Delay

    NetDeviceContainer devices = p2p.Install(nodes);

    // 3. Install Stack & IP
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 4. Install UDP Echo Server (The Receiver/ACKer) on Node 1
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 5. Install UDP Echo Client (The Sender) on Node 0
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5)); 
    
    // FIX: Reduced interval to 150ms. 
    // This allows the node to process the 100ms RTT and send the next block sequentially.
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(150))); 
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = echoClient.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // 6. NetAnim Visualization
    AnimationInterface anim("stop-and-wait.xml");
    
    // Position nodes horizontally
    anim.SetConstantPosition(nodes.Get(0), 10, 50);
    anim.SetConstantPosition(nodes.Get(1), 80, 50);

    anim.UpdateNodeDescription(nodes.Get(0), "Sender (S&W)");
    anim.UpdateNodeDescription(nodes.Get(1), "Receiver");
    anim.UpdateNodeColor(nodes.Get(0), 0, 255, 0); // Green
    anim.UpdateNodeColor(nodes.Get(1), 255, 0, 0); // Red

    anim.EnablePacketMetadata(true);

    // 7. Run
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
