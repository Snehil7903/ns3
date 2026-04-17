Hamming Code Logic 
//may contain error
#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/applications-module.h" 
#include "math.h" 
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("HammingCodeExample");
// Function to calculate residual packet error rate after Hamming (7,4) correction 
double CalculateHammingResidualError (double ber, uint32_t packetSize) 
{ 
 // Probability of 2 or more errors in a 7-bit block (Uncorrectable) 
 // P(unfixable) = 1 - [P(0 errors) + P(1 error)] 
 double p0 = pow (1 - ber, 7);
 double p1 = 7 * ber * pow (1 - ber, 6);
 double pBlockError = 1.0 - (p0 + p1);
 // Number of 4-bit data blocks in the packet 
 double numBlocks = (packetSize * 8) / 4.0;
 
 // Probability that at least one block in the packet fails 
 return 1.0 - pow (1.0 - pBlockError, numBlocks);
} 
int main (int argc, char *argv[]) 
{ 
 double ber = 0.001; // Raw Bit Error Rate (1 in 1000 bits flip)
 uint32_t packetSize = 1024;
 CommandLine cmd (__FILE__);
 cmd.AddValue ("ber", "Raw Bit Error Rate", ber);
 cmd.Parse (argc, argv);
 NodeContainer nodes;
 nodes.Create (2);
 PointToPointHelper pointToPoint;
 pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
 pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
NetDeviceContainer devices;
 devices = pointToPoint.Install (nodes);
 // --- Hamming Error Correction Simulation --- 
 double residualPER = CalculateHammingResidualError (ber, packetSize);
 
 Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
 em->SetAttribute ("ErrorRate", DoubleValue (residualPER));
 em->SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
 devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 std::cout << "Raw BER: " << ber << std::endl;
 std::cout << "Residual Packet Error Rate after Hamming (7,4): " << residualPER << 
std::endl;
 InternetStackHelper stack;
 stack.Install (nodes);
 Ipv4AddressHelper address;
 address.SetBase ("10.1.1.0", "255.255.255.0");
 Ipv4InterfaceContainer interfaces = address.Assign (devices);
 // Standard UDP Echo setup 
 UdpEchoServerHelper echoServer (9);
 ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
 serverApps.Start (Seconds (1.0));
 serverApps.Stop (Seconds (10.0));
 UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
 echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
 echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
 echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
 ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
 clientApps.Start (Seconds (2.0));
 clientApps.Stop (Seconds (10.0));
 Simulator::Run ();
 Simulator::Destroy ();
 return 0;
}