#ifndef RSCRAWLER_CRAWLER_H
#define RSCRAWLER_CRAWLER_H

#include <util/bdthreads.h>
#include <bdiface.h>

class Crawler : public bdThread {
public:
    Crawler();

    ~Crawler();

    virtual void run();

    void setRegions(int start, int end);

private:
    bdMutex crwlrMutex;
    void iteration();

    bdNodeId peerId;
    std::string bootstrapfile = "/home/ulysses/RS_NEW/RS/libbitdht/src/bitdht/bdboot.txt";
    uint16_t port = 6775;
    std::string appId = "RS51";
    BitDhtHandler* dhtHandler;
    int regionStart = 0;
    int regionEnd = 0;
};

#endif //RSCRAWLER_CRAWLER_H
