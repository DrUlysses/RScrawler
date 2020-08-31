#ifndef RSCRAWLER_CRAWLER_H
#define RSCRAWLER_CRAWLER_H

#include <util/bdthreads.h>
#include <bdiface.h>

class Crawler : public bdThread {
public:
    Crawler();
    void init();
    ~Crawler();

    virtual void run();
    virtual void stop();
    void disable();
    void enable();

    void setStage(bool stage);
    void setRegions(int start, int end);
    void setPort(uint16_t newPort);
    void extractToCheckList(std::list<bdNodeId> peers);
    std::list<bdNodeId> getToCheckList();

private:
    bdMutex crwlrMutex;
    void iterationFirstStage();
    void iterationSecondStage();

    std::list<bdNodeId> toCheckPeerList;
    bool readyToCheck = false;
    bool isAlive = true;
    bool isActive = true;
    bdNodeId* peerId;
    bool currentStage;
    std::string bootstrapfile = "/home/ulysses/RS_Gitlab/rs-crawler/libbitdht/src/bitdht/bdboot.txt";
    uint16_t port;
    std::string appId = "RS51";
    BitDhtHandler* dhtHandler;
    int regionStart = 0;
    int regionEnd = 0;
};

#endif //RSCRAWLER_CRAWLER_H
