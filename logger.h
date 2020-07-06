#ifndef RSCRAWLER_LOGGER_H
#define RSCRAWLER_LOGGER_H

#include <util/bdthreads.h>
#include <bdfilter.h>
#include <bdobj.h>

#define LOG_FILENAME "dhtlogs"
#define DHT_FILENAME "foundDHTs"
#define RS_PEERS_FILENAME "rspeers"
#define FILTERED_FILENAME "filtered"

class Logger : public bdThread {
public:
    Logger();
    ~Logger();

    virtual void run();
    virtual void stop();

    void sortRsPeers();

private:
    bool isAlive = true;
    bdMutex dhtMutex;
    std::map<bdNodeId, bdFilteredPeer> discoveredPeers;
    void iteration();

};


#endif //RSCRAWLER_LOGGER_H
