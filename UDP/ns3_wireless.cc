
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    NodeContainer nodes;
    nodes.Create(2);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
    nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(0, 0, 0));
    nodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(10, 0, 0));

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    uint16_t port = 9;

    PacketSinkHelper receiver("ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sink = receiver.Install(nodes.Get(1));
    sink.Start(Seconds(1.0));
    sink.Stop(Seconds(10.0));

    UdpClientHelper sender(interfaces.GetAddress(1), port);
    sender.SetAttribute("MaxPackets", UintegerValue(100));
    sender.SetAttribute("Interval", TimeValue(Seconds(0.1)));
    sender.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer source = sender.Install(nodes.Get(0));
    source.Start(Seconds(2.0));
    source.Stop(Seconds(9.0));

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sink.Get(0));
    std::cout << "Wireless UDP Simulation Completed\n";
    std::cout << "Packets received: "
              << packetSink->GetTotalRx() / 512 << '\n';
    std::cout << "Bytes received: "
              << packetSink->GetTotalRx() << '\n';

    Simulator::Destroy();
    return 0;
}
