#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h" // Required for NetAnim
#include "ns3/mobility-module.h" // Required to set node positions
using namespace ns3;
int main (int argc, char *argv[])
{
 uint32_t nSubnets = 5;
 uint32_t nHosts = 10;
 NodeContainer allNodes; // To keep track of all nodes for NetAnim
 InternetStackHelper stack;
 Ipv4AddressHelper address;
 PointToPointHelper p2p;
 p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
 p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
 Ipv4Mask subnetMask("255.255.255.240");
 for (uint32_t i = 0; i < nSubnets; ++i)
 {
 NodeContainer subnetNodes;
 subnetNodes.Create (nHosts);
 allNodes.Add(subnetNodes);
 stack.Install (subnetNodes);
 // Set layout: Each subnet is a new row in NetAnim
 MobilityHelper mobility;
 Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
 for (uint32_t j = 0; j < nHosts; ++j) {
 positionAlloc->Add (Vector (j * 10.0, i * 20.0, 0.0)); 
 }
 mobility.SetPositionAllocator (positionAlloc);
 mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 mobility.Install (subnetNodes);
 // Assign IP Addresses
std::stringstream ss;
 ss << "192.168.72." << (i * 16);
 address.SetBase (ss.str ().c_str (), subnetMask);
 for (uint32_t j = 0; j < nHosts - 1; ++j) {
 NetDeviceContainer d = p2p.Install (subnetNodes.Get(j), subnetNodes.Get(j+1));
 address.Assign (d);
 }
 }
 // --- NetAnim Configuration ---
 // This creates the XML file that NetAnim reads
 AnimationInterface anim ("subnet_simulation.xml"); 
 // Optional: Label the nodes in NetAnim
 for (uint32_t i = 0; i < allNodes.GetN(); ++i) {
 anim.UpdateNodeDescription(allNodes.Get(i), "Host-" + std::to_string(i));
 }
 Simulator::Run ();
 Simulator::Destroy ();
 return 0;
}

This is a error code....