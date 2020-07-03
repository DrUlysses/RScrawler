#ifndef RSCRAWLER_LOGGER_H
#define RSCRAWLER_LOGGER_H

#include <util/bdthreads.h>
#include <bdfilter.h>
#include <bdobj.h>

#define LOG_FILENAME "dhtlogs"
#define DHT_FILENAME "foundDHTs"
#define RS_PEERS_FILENAME "rspeers"

class Logger : public bdThread {
public:
    Logger();
    ~Logger();

    virtual void run();

    void sortRsPeers();

private:
    bdMutex dhtMutex;
    std::map<bdNodeId, bdFilteredPeer> discoveredPeers;
    //static std::map<bdId, bdToken> RSPeers;
    void iteration();
};


#endif //RSCRAWLER_LOGGER_H
