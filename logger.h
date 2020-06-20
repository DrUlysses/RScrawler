#ifndef RSCRAWLER_LOGGER_H
#define RSCRAWLER_LOGGER_H

#include <util/bdthreads.h>
#include <bdfilter.h>

#define LOG_FILENAME "dhtlogs"
#define DHT_FILENAME "foundDHTs"

class Logger : public bdThread {
public:
    Logger();
    ~Logger();

    virtual void run();
    //int tick();

private:
    bdMutex dhtMutex;
    std::map<bdNodeId, bdFilteredPeer> discoveredPeers;
    void iteration();
};


#endif //RSCRAWLER_LOGGER_H
