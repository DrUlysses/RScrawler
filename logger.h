#ifndef RSCRAWLER_LOGGER_H
#define RSCRAWLER_LOGGER_H

#include <util/bdthreads.h>
#include <bdfilter.h>
#include <bdobj.h>
#include <vector>

#define LOG_FILENAME "dhtlogs"
#define DHT_FILENAME "foundDHTs"
#define RS_PEERS_FILENAME "rspeers"
#define FILTERED_FILENAME "filtered"

class Logger : public bdThread {
public:
    Logger();
    ~Logger();

    virtual void run();
    void disable();
    void enable();
    void stop();

    void sortRsPeers(std::list<bdId>* result = nullptr);
    std::list<bdNodeId> getDiscoveredPeers();

    bdMutex dhtMutex;
private:
    bool isAlive = true;
    bool isActive = true;
    bool isSecondStage = false;
    static std::map<bdNodeId, bdFilteredPeer> discoveredPeers;

    void iteration();
};


#endif //RSCRAWLER_LOGGER_H
