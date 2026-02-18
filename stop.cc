#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StopAndWaitSequenceExample");

/* =======================
   Sender Application
   ======================= */

class StopWaitSender : public Application
{
public:
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
  virtual void StartApplication(void)
  {
    m_socket->Connect(m_peer);
    m_socket->SetRecvCallback(
        MakeCallback(&StopWaitSender::ReceiveAck, this));
    SendPacket();
  }

  void SendPacket()
  {
    if (m_packetsSent < m_pktCount)
    {
      Ptr<Packet> packet = Create<Packet>(1024);

      NS_LOG_UNCOND("Sender: Sending Pkt Seq "
                    << m_seq << " at "
                    << Simulator::Now().GetSeconds() << "s");

      m_socket->Send(packet);

      m_timeoutEvt = Simulator::Schedule(
          m_timeout,
          &StopWaitSender::SendPacket,
          this);
    }
  }

  void ReceiveAck(Ptr<Socket> socket)
  {
    Ptr<Packet> packet = socket->Recv();

    m_timeoutEvt.Cancel();

    NS_LOG_UNCOND("Sender: Received ACK for Seq "
                  << m_seq);

    m_seq = 1 - m_seq;
    m_packetsSent++;

    if (m_packetsSent < m_pktCount)
    {
      Simulator::ScheduleNow(
          &StopWaitSender::SendPacket,
          this);
    }
  }

  Ptr<Socket> m_socket;
  Address m_peer;
  uint8_t m_seq;
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
  StopWaitReceiver()
    : m_socket(0),
      m_expectedSeq(0) {}

  void Setup(Ptr<Socket> socket)
  {
    m_socket = socket;
  }

private:
  virtual void StartApplication(void)
  {
    m_socket->Listen();
    m_socket->SetRecvCallback(
        MakeCallback(&StopWaitReceiver::HandleRead, this));
  }

  void HandleRead(Ptr<Socket> socket)
  {
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
      NS_LOG_UNCOND("Receiver: Received Packet. Sending ACK...");

      Ptr<Packet> ack = Create<Packet>(10);
      socket->SendTo(ack, 0, from);
    }
  }

  Ptr<Socket> m_socket;
  uint8_t m_expectedSeq;
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

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t port = 8080;

  /* Receiver */
  Ptr<Socket> recvSocket =
      Socket::CreateSocket(nodes.Get(1),
                           UdpSocketFactory::GetTypeId());
  InetSocketAddress local =
      InetSocketAddress(Ipv4Address::GetAny(), port);
  recvSocket->Bind(local);

  Ptr<StopWaitReceiver> receiver =
      CreateObject<StopWaitReceiver>();
  receiver->Setup(recvSocket);
  nodes.Get(1)->AddApplication(receiver);
  receiver->SetStartTime(Seconds(0.0));
  receiver->SetStopTime(Seconds(20.0));

  /* Sender */
  Ptr<Socket> sendSocket =
      Socket::CreateSocket(nodes.Get(0),
                           UdpSocketFactory::GetTypeId());

  Ptr<StopWaitSender> sender =
      CreateObject<StopWaitSender>();
  sender->Setup(sendSocket,
                InetSocketAddress(interfaces.GetAddress(1), port),
                Seconds(1.0));
  nodes.Get(0)->AddApplication(sender);
  sender->SetStartTime(Seconds(1.0));
  sender->SetStopTime(Seconds(20.0));

  /* NetAnim */
  AnimationInterface anim("stopwait.xml");
  anim.SetConstantPosition(nodes.Get(0), 10, 20);
  anim.SetConstantPosition(nodes.Get(1), 50, 20);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}