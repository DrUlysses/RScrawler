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
    void disable();
    void enable();
    void stop();

    void sortRsPeers(std::list<bdId>* result = nullptr);
    static std::list<bdNodeId> getDiscoveredPeers();

    bdMutex dhtMutex;
private:
    bool isAlive = true;
    bool isActive = true;
    bool isSecondStage = false;
    static std::map<bdNodeId, bdFoundPeer> discoveredPeers;

    void iteration();
};


#endif //RSCRAWLER_LOGGER_H
