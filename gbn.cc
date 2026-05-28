#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/applications-module.h" 
#include "ns3/netanim-module.h" 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("GoBackNExample");

int main (int argc, char *argv[]) 
{ 
    // Allow overriding parameters from command line 
    uint32_t payloadSize = 1448;
    std::string dataRate = "5Mbps";
    std::string delay = "2ms";
    
    CommandLine cmd (__FILE__);
    cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
    cmd.Parse (argc, argv);

    // Disable SACK to make TCP behave more like standard Go-Back-N
    Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (false));

    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
    pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    // Populate routing tables (good practice, even for direct P2P links)
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // --- Receiver Setup (Node 1) ---
    uint16_t port = 8080;
    Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), port));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (1));
    sinkApp.Start (Seconds (1.0));
    sinkApp.Stop (Seconds (10.0));

    // --- Sender Setup (Node 0) --- 
    // BulkSend will continuously send data to fill the window 
    BulkSendHelper source ("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute ("MaxBytes", UintegerValue (0)); // 0 means unlimited
    source.SetAttribute ("SendSize", UintegerValue (payloadSize));
    ApplicationContainer sourceApp = source.Install (nodes.Get (0));
    sourceApp.Start (Seconds (2.0));
    sourceApp.Stop (Seconds (10.0));

    // --- Error Model Setup --- 
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    
    // FIX: Explicitly set the unit to PACKETS instead of the default BYTES
    em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET")); 
    
    // Set the error rate (0.01 means 1% of packets will be dropped) 
    em->SetAttribute ("ErrorRate", DoubleValue (0.01)); 

    // Attach the error model to Node 1's receiving device 
    devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    // --- Animation --- 
    AnimationInterface anim ("gbn-animation.xml");
    anim.SetConstantPosition (nodes.Get (0), 10.0, 10.0);
    anim.SetConstantPosition (nodes.Get (1), 60.0, 10.0);
    anim.EnablePacketMetadata (true); // Enable packet metadata to see sequence numbers

    Simulator::Run ();
    Simulator::Destroy ();
    
    return 0;
}
