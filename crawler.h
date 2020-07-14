#ifndef RSCRAWLER_CRAWLER_H
#define RSCRAWLER_CRAWLER_H

#include <util/bdthreads.h>
#include <bdiface.h>

class Crawler : public bdThread {
public:
    Crawler();

    ~Crawler();

    virtual void run();
    virtual void stop();
    void disable();
    void enable();

    void setStage(bool stage);
    void setRegions(int start, int end);
    void extractToCheckList(std::list<bdNodeId> peers);
    std::list<bdNodeId> getToCheckList();

private:
    bdMutex crwlrMutex;
    void iterationFirstStage();
    void iterationSecondStage();

    std::list<bdNodeId> toCheckPeerList;
    bool readyToCheck = false;
    bool isAlive = true;
    bdNodeId peerId;
    bool currentStage;
    std::string bootstrapfile = "/home/ulysses/RS_NEW/RS/libbitdht/src/bitdht/bdboot.txt";
    uint16_t port = 6775;
    std::string appId = "RS51";
    BitDhtHandler* dhtHandler;
    int regionStart = 0;
    int regionEnd = 0;
};

#endif //RSCRAWLER_CRAWLER_H
