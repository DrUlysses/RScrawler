#ifndef RSCRAWLER_LOGGER_H
#define RSCRAWLER_LOGGER_H

#include <util/bdthreads.h>
#include <bdfilter.h>
#include <bdobj.h>
#include <vector>

#define LOG_FILENAME "dhtlogs"
#define RS_PEERS_FILENAME "rspeers"
#define TICK_PAUSE	5
#define USED_IDS_FILENAME "my_ids"

class bdFoundPeer {
public:
    struct sockaddr_in mAddr;
    std::string mVersion;
    time_t mLastSeen;
};

class Logger : public bdThread {
public:
    Logger();
    ~Logger();

    virtual void run();
    void enable(bool state);
    void stop();

    static std::list<bdNodeId> getDiscoveredRSPeers();
    static std::list<bdNodeId> getDiscoveredPeers();

    bdMutex dhtMutex;
private:
    bool isAlive = true;
    bool isActive = false;
    static std::map<bdNodeId, bdFoundPeer> discoveredPeers;
    static std::map<bdNodeId, bdFoundPeer> discoveredRSPeers;

    void iteration();
};


#endif //RSCRAWLER_LOGGER_H
