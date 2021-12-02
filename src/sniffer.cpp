#include "../includes/sniffer.h"

static long long traffAll = 0;
static long long trafPerSec = 0;
static long long packsAll = 0;
static long long packsPerSec = 0;

static std::mutex trafficMutex;

/*
	Local function
*/

static void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	const struct SniffIp *ip;
	ip = (struct SniffIp *)(packet + SIZE_ETHERNET);
	
	trafficMutex.lock();

	packsPerSec++;
	packsAll++;
	traffAll += ntohs(ip->ip_len);
	trafPerSec += ntohs(ip->ip_len);

	trafficMutex.unlock();

	return;
}

/*
	Non-class functions
*/

std::string bytesToString(long long bytes)
{
	float bytesTemp = (float)bytes;
	std::string str = "<empty>";

	if (bytesTemp < 1024)
	{
		str = std::to_string(bytesTemp);
		str = str.substr(0, str.size() - 4) + " B";
	}
	else if (bytesTemp >= 1024 && bytesTemp < pow(1024, 2))
	{
		bytesTemp = (float)bytesTemp / 1024;
		str = std::to_string(bytesTemp);
		str = str.substr(0, str.size() - 4) + " KB";
	}
	else if (bytesTemp >= pow(1024, 2) && bytesTemp < pow(1024, 3))
	{
		bytesTemp = (float)bytesTemp / pow(1024, 2);
		str = std::to_string(bytesTemp);
		str = str.substr(0, str.size() - 4) + " MB";
	}
	else if (bytesTemp >= pow(1024, 3))
	{
		bytesTemp = (float)bytesTemp / pow(1024, 3);
		str = std::to_string(bytesTemp);
		str = str.substr(0, str.size() - 4) + " GB";
	}
	else
	{
		str = std::to_string(bytesTemp);
		str = str.substr(0, str.size() - 4) + " B";
	}

	return str;
}

std::string packetsToString(long long packets)
{
	std::string str = "<empty>";

	if (packets < 1000)
	{
		str = std::to_string(packets);
	}
	else if (packets >= 1000 && packets < pow(1000, 2))
	{
		packets /= 1000;
		str = std::to_string(packets) + " k";
	}
	else if (packets >= pow(1000, 2) && packets < pow(1000, 3))
	{
		packets /= pow(1000, 2);
		str = std::to_string(packets) + " m";
	}
	else if (packets >= pow(1000, 3))
	{
		packets /= pow(1000, 3);
		str = std::to_string(packets) + " b";
	}
	else
	{
		str = std::to_string(packets);
	}

	return str;
}

/*
	Public methods
*/

Sniffer::Sniffer(const std::string &dev)
{
	this->device = dev;
	this->filter_exp = "";
	this->num_packets = -1;

	this->trafficAll = 0;
	this->trafficPerSec = 0;
	this->packetsAll = 0;
	this->packetsPerSec = 0;

	char errbuf[PCAP_ERRBUF_SIZE];

	if (pcap_lookupnet(this->device.c_str(), &this->net, &this->mask, errbuf) == -1)
	{
		message(ERROR_M, "Sniffer: couldn't get netmask for device " + this->device + ": " + errbuf);
		this->net = 0;
		this->mask = 0;
	}
	message(NOTICE_M, "Sniffer: device: " + this->device);

	if (this->filter_exp != "")
	{
		message(NOTICE_M, "Sniffer: filter expression: " + this->filter_exp);
	}
	else
	{
		message(NOTICE_M, "Sniffer: filter expression: no filter");
	}

	this->handle = pcap_open_live(this->device.c_str(), SNAP_LEN, 1, 1000, errbuf);
	if (this->handle == NULL)
	{
		message(ERROR_M, "Sniffer: couldn't open device " + this->device + ": " + errbuf);
		message(ERROR_M, "Sniffer: initialization failed");
		std::raise(SIGINT);
	}

	if (pcap_datalink(this->handle) != DLT_EN10MB)
	{
		message(ERROR_M, "Sniffer: " + this->device + " is not an Ethernet");
		message(ERROR_M, "Sniffer: initialization failed");
		std::raise(SIGINT);
	}

	pcap_setdirection(this->handle, PCAP_D_IN);

	if (pcap_compile(this->handle, &this->fp, this->filter_exp.c_str(), 0, this->net) == -1)
	{
		message(ERROR_M, "Sniffer: couldn't parse filter " + this->filter_exp + ": " + pcap_geterr(this->handle));
		message(ERROR_M, "Sniffer: initialization failed");
		std::raise(SIGINT);
	}

	if (pcap_setfilter(this->handle, &this->fp) == -1)
	{
		message(ERROR_M, "Sniffer: couldn't install filter " + this->filter_exp + ": " + pcap_geterr(this->handle));
		message(ERROR_M, "Sniffer: initialization failed");
		std::raise(SIGINT);
	}

	message(NOTICE_M, "Sniffer: initialization completed");

	return;
}

Sniffer::~Sniffer()
{
	message(NOTICE_M, "Sniffer: total processed " + bytesToString(traffAll));
	message(NOTICE_M, "Sniffer: total processed " + packetsToString(packsAll) + " packets");
	message(NOTICE_M, "Sniffer: end of sniffer");

	return;
}

void Sniffer::sniff(Timer &timer)
{
	timer.setTimeStartSniffing();

	std::this_thread::sleep_until(timer.getTimeCutoff(SNIFFER_CUTOFF_F));
	timer.setTimeCutoff(SNIFFER_CUTOFF_F);
	message(NOTICE_M, "Start of sniffing ...");

	pcap_loop(this->handle, this->num_packets, got_packet, NULL);

	std::this_thread::sleep_until(timer.getTimeCutoff(TRAFFIC_COUNTER_CUTOFF_F));
	message(NOTICE_M, "End of sniffing");

	return;
}

void Sniffer::countTraffic(Timer &timer)
{
	std::this_thread::sleep_until(timer.getTimeCutoff(TRAFFIC_COUNTER_CUTOFF_F));
	timer.setTimeCutoff(TRAFFIC_COUNTER_CUTOFF_F);
	message(NOTICE_M, "Start of traffic counting ...");

	traffAll = 0;
	trafPerSec = 0;
	packsAll = 0;
	packsPerSec = 0;

	int interruptionCounter = 0;

	while (interruptionCounter < NUMBER_CUTOFF_F - TRAFFIC_COUNTER_CUTOFF_F)
	{
		if (interruptFlag)
		{
			interruptionCounter++;
		}
		
		std::this_thread::sleep_until(timer.getTimeCutoff(TRAFFIC_COUNTER_CUTOFF_F));
		timer.setTimeCutoff(TRAFFIC_COUNTER_CUTOFF_F);

		trafficMutex.lock();

		this->trafficAll = traffAll;
		this->trafficPerSec = trafPerSec;
		this->packetsAll = packsAll;
		this->packetsPerSec = packsPerSec;
		trafPerSec = 0;
		packsPerSec = 0;

		trafficMutex.unlock();
	}

	this->trafficPerSec = trafPerSec;
	this->packetsPerSec = packsPerSec;

	message(NOTICE_M, "End of traffic counting");


	pcap_breakloop(this->handle);
	pcap_freecode(&this->fp);
	pcap_close(this->handle);
	
	return;
}

std::string Sniffer::getDevice()
{
	return this->device;
}

long long Sniffer::getTrafficAll()
{
	return this->trafficAll;
}

long long Sniffer::getTrafficPerSec()
{
	return this->trafficPerSec;
}

long long Sniffer::getPacketsPerSec()
{
	return this->packetsPerSec;
}

long long Sniffer::getPacketsAll()
{
	return this->packetsAll;
}