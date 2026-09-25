#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/seq-ts-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StopAndWaitSequenceExample");

/* ========================================================================
   Sender Application
   ======================================================================== */
class StopWaitSender : public Application
{
public:
  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("StopWaitSender")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<StopWaitSender>();
    return tid;
  }

  StopWaitSender() = default;

  void Setup(Ptr<Socket> socket, Address address, Time timeout)
  {
    m_socket = socket;
    m_peer = address;
    m_timeout = timeout;
  }

private:
  void StartApplication() override
  {
    if (m_socket)
    {
      m_socket->Connect(m_peer);
      m_socket->SetRecvCallback(MakeCallback(&StopWaitSender::ReceiveAck, this));
    }
    SendPacket();
  }

  void StopApplication() override
  {
    if (m_timeoutEvt.IsRunning())
    {
      m_timeoutEvt.Cancel();
    }

    if (m_socket)
    {
      m_socket->Close();
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
  }

  void SendPacket()
  {
    if (m_packetsSent < m_pktCount)
    {
      Ptr<Packet> packet = Create<Packet>(1024);

      SeqTsHeader seqHeader;
      seqHeader.SetSeq(m_seq);
      packet->AddHeader(seqHeader);

      NS_LOG_UNCOND("Sender: Sending Pkt Seq " << m_seq << " at " << Simulator::Now().GetSeconds() << "s");
      m_socket->Send(packet);

      if (m_timeoutEvt.IsRunning())
      {
        m_timeoutEvt.Cancel(); 
      }
      m_timeoutEvt = Simulator::Schedule(m_timeout, &StopWaitSender::SendPacket, this);
    }
  }

  void ReceiveAck(Ptr<Socket> socket)
  {
    Ptr<Packet> packet;
    while ((packet = socket->Recv()))
    {
      SeqTsHeader ackHeader;
      if (packet->RemoveHeader(ackHeader) == 0) 
      {
        continue; 
      }
      
      if (ackHeader.GetSeq() == m_seq)
      {
        if (m_timeoutEvt.IsRunning())
        {
          m_timeoutEvt.Cancel();
        }

        NS_LOG_UNCOND("Sender: Received ACK for Seq " << m_seq << " at " << Simulator::Now().GetSeconds() << "s");

        m_seq = 1 - m_seq; 
        m_packetsSent++;

        if (m_packetsSent < m_pktCount)
        {
          Simulator::ScheduleNow(&StopWaitSender::SendPacket, this);
        }
      }
      else
      {
         NS_LOG_UNCOND("Sender: Ignored invalid/duplicate ACK for Seq " << ackHeader.GetSeq());
      }
    }
  }

  Ptr<Socket> m_socket{nullptr};
  Address m_peer;
  uint32_t m_seq{0}; 
  uint32_t m_pktCount{10};
  uint32_t m_packetsSent{0};
  Time m_timeout{Seconds(1.0)};
  EventId m_timeoutEvt;
};

/* ========================================================================
   Receiver Application
   ======================================================================== */
class StopWaitReceiver : public Application
{
public:
  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("StopWaitReceiver")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<StopWaitReceiver>();
    return tid;
  }

  StopWaitReceiver() = default;

  void Setup(Ptr<Socket> socket)
  {
    m_socket = socket;
  }

private:
  void StartApplication() override
  {
    if (m_socket)
    {
      m_socket->SetRecvCallback(MakeCallback(&StopWaitReceiver::HandleRead, this));
    }
  }

  void StopApplication() override
  {
    if (m_socket)
    {
      m_socket->Close();
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
  }

  void HandleRead(Ptr<Socket> socket)
  {
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
      SeqTsHeader seqHeader;
      if (packet->RemoveHeader(seqHeader) == 0)
      {
        continue;
      }
      uint32_t recvSeq = seqHeader.GetSeq();

      if (recvSeq == m_expectedSeq)
      {
        NS_LOG_UNCOND("Receiver: Received expected Packet Seq " << recvSeq << ".");
        m_expectedSeq = 1 - m_expectedSeq; 
      }
      else
      {
        NS_LOG_UNCOND("Receiver: Received DUPLICATE Packet Seq " << recvSeq << ". Discarding payload.");
      }

      NS_LOG_UNCOND("Receiver: Sending ACK for Seq " << recvSeq << "...");
      Ptr<Packet> ack = Create<Packet>(10);
      SeqTsHeader ackHeader;
      ackHeader.SetSeq(recvSeq);
      ack->AddHeader(ackHeader);

      socket->SendTo(ack, 0, from);
    }
  }

  Ptr<Socket> m_socket{nullptr};
  uint32_t m_expectedSeq{0}; 
};

/* ========================================================================
   Main Execution
   ======================================================================== */
int main(int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices = p2p.Install(nodes);

  auto em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(0.15)); 
  em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  constexpr uint16_t port = 8080;

  // Setup Receiver
  Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(1), UdpSocketFactory::GetTypeId());
  recvSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));

  auto receiver = CreateObject<StopWaitReceiver>();
  receiver->Setup(recvSocket);
  nodes.Get(1)->AddApplication(receiver);
  receiver->SetStartTime(Seconds(0.0));
  receiver->SetStopTime(Seconds(20.0));

  // Setup Sender
  Ptr<Socket> sendSocket = Socket::CreateSocket(nodes.Get(0), UdpSocketFactory::GetTypeId());

  auto sender = CreateObject<StopWaitSender>();
  sender->Setup(sendSocket, InetSocketAddress(interfaces.GetAddress(1), port), Seconds(1.0));
  nodes.Get(0)->AddApplication(sender);
  sender->SetStartTime(Seconds(1.0));
  sender->SetStopTime(Seconds(20.0));

  // Visualizer / NetAnim Output Configuration
  AnimationInterface anim("stopwait.xml");
  anim.SetConstantPosition(nodes.Get(0), 10.0, 20.0);
  anim.SetConstantPosition(nodes.Get(1), 50.0, 20.0);
  
  anim.UpdateNodeDescription(nodes.Get(0), "Sender");
  anim.UpdateNodeDescription(nodes.Get(1), "Receiver");
  anim.EnablePacketMetadata(true);

  Simulator::Run();
  Simulator::Destroy();
  
  return 0;
}
