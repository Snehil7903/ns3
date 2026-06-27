#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/seq-ts-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StopAndWaitSequenceExample");

/* =======================
   Sender Application
   ======================= */

class StopWaitSender : public Application
{
public:
  static TypeId GetTypeId(void)
  {
    static TypeId tid = TypeId("StopWaitSender")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<StopWaitSender>();
    return tid;
  }

  StopWaitSender()
      : m_socket(0),
        m_seq(0),
        m_pktCount(10),
        m_packetsSent(0) {}

  void Setup(Ptr<Socket> socket, Address address, Time timeout)
  {
    m_socket = socket;
    m_peer = address;
    m_timeout = timeout;
  }

private:
  virtual void StartApplication(void) override
  {
    m_socket->Connect(m_peer);
    m_socket->SetRecvCallback(MakeCallback(&StopWaitSender::ReceiveAck, this));
    SendPacket();
  }

  virtual void StopApplication(void) override
  {
    // FIX 1: Must cancel pending events before closing the socket to avoid segfaults
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
    
    // FIX 3: Wrapped Recv() in a while loop to drain the socket buffer fully
    while ((packet = socket->Recv()))
    {
      SeqTsHeader ackHeader;
      packet->RemoveHeader(ackHeader);
      
      if (ackHeader.GetSeq() == m_seq)
      {
        m_timeoutEvt.Cancel();

        NS_LOG_UNCOND("Sender: Received ACK for Seq " << m_seq);

        m_seq = 1 - m_seq; // Toggle sequence between 0 and 1
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

  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_seq; // FIX 4: Changed to uint32_t to match SeqTsHeader
  uint32_t m_pktCount;
  uint32_t m_packetsSent;
  Time m_timeout;
  EventId m_timeoutEvt;
};

/* =======================
   Receiver Application
   ======================= */

class StopWaitReceiver : public Application
{
public:
  static TypeId GetTypeId(void)
  {
    static TypeId tid = TypeId("StopWaitReceiver")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<StopWaitReceiver>();
    return tid;
  }

  StopWaitReceiver()
      : m_socket(0),
        m_expectedSeq(0) {}

  void Setup(Ptr<Socket> socket)
  {
    m_socket = socket;
  }

private:
  virtual void StartApplication(void) override
  {
    m_socket->Listen();
    m_socket->SetRecvCallback(MakeCallback(&StopWaitReceiver::HandleRead, this));
  }

  virtual void StopApplication(void) override
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
      packet->RemoveHeader(seqHeader);
      uint32_t recvSeq = seqHeader.GetSeq();

      // FIX 2: Actually check if the received sequence matches what we expect
      if (recvSeq == m_expectedSeq)
      {
        NS_LOG_UNCOND("Receiver: Received expected Packet Seq " << recvSeq << ".");
        m_expectedSeq = 1 - m_expectedSeq; // Toggle expected sequence
      }
      else
      {
        NS_LOG_UNCOND("Receiver: Received DUPLICATE Packet Seq " << recvSeq << ". Discarding payload.");
      }

      // We ALWAYS send an ACK for the received sequence, even if it was a duplicate
      NS_LOG_UNCOND("Receiver: Sending ACK for Seq " << recvSeq << "...");
      Ptr<Packet> ack = Create<Packet>(10);
      SeqTsHeader ackHeader;
      ackHeader.SetSeq(recvSeq);
      ack->AddHeader(ackHeader);

      socket->SendTo(ack, 0, from);
    }
  }

  Ptr<Socket> m_socket;
  uint32_t m_expectedSeq; // FIX 4: Changed to uint32_t
};

/* =======================
            Main
   ======================= */

int main(int argc, char *argv[])
{
  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices = p2p.Install(nodes);

  // OPTIONAL ADDITION: Adding an Error Model to actually drop packets and test retransmissions
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(0.15)); // 15% drop rate
  em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t port = 8080;

  /* Receiver */
  Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(1), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
  recvSocket->Bind(local);

  Ptr<StopWaitReceiver> receiver = CreateObject<StopWaitReceiver>();
  receiver->Setup(recvSocket);
  nodes.Get(1)->AddApplication(receiver);
  receiver->SetStartTime(Seconds(0.0));
  receiver->SetStopTime(Seconds(20.0));

  /* Sender */
  Ptr<Socket> sendSocket = Socket::CreateSocket(nodes.Get(0), UdpSocketFactory::GetTypeId());

  Ptr<StopWaitSender> sender = CreateObject<StopWaitSender>();
  sender->Setup(sendSocket, InetSocketAddress(interfaces.GetAddress(1), port), Seconds(1.0));
  
  nodes.Get(0)->AddApplication(sender);
  sender->SetStartTime(Seconds(1.0));
  sender->SetStopTime(Seconds(20.0));

  /* NetAnim */
  AnimationInterface anim("stopwait.xml");
  anim.SetConstantPosition(nodes.Get(0), 10, 20);
  anim.SetConstantPosition(nodes.Get(1), 50, 20);
  anim.EnablePacketMetadata(true);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
