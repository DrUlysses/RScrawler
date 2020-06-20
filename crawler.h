#ifndef RSCRAWLER_CRAWLER_H
#define RSCRAWLER_CRAWLER_H

#include <util/bdthreads.h>
#include <bdiface.h>

class Crawler : public bdThread {
public:
    Crawler();
    ~Crawler();

    virtual void run();

private:
    bdMutex crwlrMutex;
    void iteration();

    bdNodeId peerId;
    std::string bootstrapfile = "/home/ulysses/RS_NEW/RS/libbitdht/src/bitdht/bdboot.txt";
    std::string ip;
    uint16_t port;
};

#endif //RSCRAWLER_CRAWLER_H
