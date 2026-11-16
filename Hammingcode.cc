#include <iostream>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HammingCodeExample");

// Calculate residual packet error rate after Hamming (7,4) correction
double CalculateHammingResidualError (double ber, uint32_t totalFrameSize)
{
    if (ber <= 0.0)
    {
        return 0.0;
    }
    if (ber >= 1.0)
    {
        return 1.0;
    }

    // Probability of 2 or more errors in a 7-bit block (uncorrectable by Hamming)
    double p0 = std::pow (1.0 - ber, 7);
    double p1 = 7.0 * ber * std::pow (1.0 - ber, 6);
    double pBlockError = 1.0 - (p0 + p1);

    // Total data bits mapped to 4-bit data blocks (ceiling division)
    uint32_t totalBits = totalFrameSize * 8;
    uint32_t numBlocks = (totalBits + 3) / 4;

    // Numerically stable computation of 1 - (1 - pBlockError)^numBlocks
    return -std::expm1 (static_cast<double> (numBlocks) * std::log1p (-pBlockError));
}

int main (int argc, char *argv[])
{
    double ber = 0.001; // Raw Bit Error Rate
    uint32_t payloadSize = 1024;
    uint32_t overheadSize = 30; // IP (20) + UDP (8) + PPP (2)

    CommandLine cmd (__FILE__);
    cmd.AddValue ("ber", "Raw Bit Error Rate", ber);
    cmd.AddValue ("payloadSize", "UDP Payload Size", payloadSize);
    cmd.Parse (argc, argv);

    // Enable application logs to observe sent, received, and dropped packets
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    uint32_t frameSize = payloadSize + overheadSize;

    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer devices = pointToPoint.Install (nodes);

    // --- Hamming Error Correction Simulation ---
    double residualPER = CalculateHammingResidualError (ber, frameSize);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (residualPER));
    em->SetAttribute ("ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
    em->AssignStreams (1); // Deterministic RNG stream

    devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    std::cout << "Raw BER: " << ber << std::endl;
    std::cout << "Total Unencoded Frame Size: " << frameSize << " bytes" << std::endl;
    std::cout << "Residual Packet Error Rate after Hamming (7,4): " << residualPER << std::endl;

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (12.0));

    UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

    ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (12.0));

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
