#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
    uint32_t nSubnets = 5;
    uint32_t nHosts = 10;

    NodeContainer router;
    router.Create(1);

    InternetStackHelper stack;
    stack.Install(router);

    std::vector<NodeContainer> subnetNodes(nSubnets);
    std::vector<NetDeviceContainer> devices(nSubnets);
    std::vector<Ipv4InterfaceContainer> interfaces(nSubnets);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    Ipv4AddressHelper address;
    Ipv4Mask mask("255.255.255.240");

    // Mobility for visualization
    MobilityHelper mobility;

    for (uint32_t i = 0; i < nSubnets; ++i)
    {
        subnetNodes[i].Create(nHosts);
        stack.Install(subnetNodes[i]);

        NodeContainer network;
        network.Add(router.Get(0));
        network.Add(subnetNodes[i]);

        devices[i] = csma.Install(network);

        std::stringstream ss;
        ss << "192.168.72." << (i * 16);
        address.SetBase(ss.str().c_str(), mask);

        interfaces[i] = address.Assign(devices[i]);

        // Positioning for NetAnim
        Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
        pos->Add(Vector(50, i * 30, 0)); // router position

        for (uint32_t j = 0; j < nHosts; ++j)
        {
            pos->Add(Vector(150 + j * 20, i * 30, 0));
        }

        mobility.SetPositionAllocator(pos);
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(network);
    }

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ---- Ping Application ----
    // Ping from subnet 1 host 0 → subnet 5 host 0

    V4PingHelper ping(interfaces[4].GetAddress(1)); // target IP
    ping.SetAttribute("Verbose", BooleanValue(true));

    ApplicationContainer app = ping.Install(subnetNodes[0].Get(0));
    app.Start(Seconds(2.0));
    app.Stop(Seconds(10.0));

    // ---- NetAnim ----
    AnimationInterface anim("five_subnets.xml");

    Simulator::Stop(Seconds(12.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}