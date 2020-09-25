#ifndef RSCRAWLER_CRAWLER_H
#define RSCRAWLER_CRAWLER_H

#include <util/bdthreads.h>
#include <bdiface.h>

#define TICK_PAUSE 5 // in seconds
#define SEARCH_SHOTS_COUNT 5
#define SEARCH_ROUNDS_COUNT 3
#define USED_IDS_FILENAME "my_ids"
#define PATH_TO_BDBOOT "/home/ulysses/RS_Gitlab/rs-crawler/libbitdht/src/bitdht/bdboot.txt"

class Crawler : public bdThread {
public:
    Crawler();
    ~Crawler();
    void initDhtHandler();
    void init();

    virtual void run();
    virtual void stop();
    void restart();
    void start();
    void enable(bool state);
    void setActive(bool state);
    bool getActive();

    void setStage(bool stage);
    void setRegions(int start, int end);
    void setPort(uint16_t newPort);
    void setCrawlsCount(unsigned int count);
    void extractToCheckList(std::list<bdNodeId> peers);
    void writeLogs();
    std::list<bdNodeId> getToCheckList();

private:
    bdMutex crwlrMutex;
    void iterationFirstStage();
    void iterationSecondStage();

    std::list<bdNodeId> toCheckPeerList;
    unsigned int crawlsCount = 1;
    bool readyToCheck = false;
    bool isActive = true;
    bdNodeId* peerId;
    bool currentStage;
    std::string bootstrapfile = PATH_TO_BDBOOT;
    uint16_t port;
    std::string appId = "RS51";
    BitDhtHandler* dhtHandler;
    int regionStart = 0;
    int regionEnd = 0;
};

#endif //RSCRAWLER_CRAWLER_H
