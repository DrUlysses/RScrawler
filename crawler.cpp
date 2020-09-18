#include <zconf.h>
#include <bdstddht.h>
#include <bootstrap_fn.h>
#include <cstring>
#include "crawler.h"

Crawler::Crawler() : crwlrMutex(true) {}

void Crawler::init() {
    bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
    peerId = new bdNodeId();
    std::cerr << "Crawler object created\n";
    //peerId = RsPeerId::random().toStdString();
    bdStdRandomNodeId(peerId);
    std::cerr << "Starting with ownID: ";
    bdStdPrintNodeId(std::cerr, peerId);
    std::cerr << std::endl;
    FILE *myIDs = fopen(USED_IDS_FILENAME, "a+");
    std::string stringID;
    bdStdPrintNodeId(stringID, peerId, false);
    fprintf(myIDs, "%s\n", stringID.c_str());
    fclose(myIDs);
}

Crawler::~Crawler() noexcept {
    std::cerr << "Crawler object destroyed, id: ";
    bdStdPrintNodeId(std::cerr, peerId);
    std::cerr << std::endl;
}

void Crawler::stop() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->shutdown();
    }
}

void Crawler::start() {
    bdThread::start();
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->start();
    }
}

void Crawler::restart() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->restart();
    }
}

void Crawler::enable(bool state) {
    isActive = state;
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->enable(state);
    }
}

void Crawler::setActive(bool state) {
    isActive = state;
}

void Crawler::setCrawlsCount(unsigned int count) {
    crawlsCount = count;
}

void Crawler::run() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler = new BitDhtHandler(peerId, port, appId, bootstrapfile);
    }
    for (unsigned int i = 0; i < crawlsCount; i++) {
        if (isActive) {
            if (currentStage == 0) {
                bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
                Crawler::iterationFirstStage();
                writeLogs();
            } else {
                bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
                Crawler::iterationSecondStage();
                writeLogs();
            }
        }
        sleep(TICK_PAUSE);
    }
}

void Crawler::iterationFirstStage() {
    // Start random requests from separate threads
    if (!readyToCheck)
        bdSingleSourceFindPeers(*dhtHandler, SEARCH_SHOTS_COUNT, SEARCH_ROUNDS_COUNT, regionStart, regionEnd, toCheckPeerList);
    else {
        bdCheckPeersFromList(*dhtHandler, toCheckPeerList);
        readyToCheck = false;
    }
}

void Crawler::iterationSecondStage() {
    if (!toCheckPeerList.empty()) {
        time_t tempTime = time(NULL);
        bdFindPeers(*dhtHandler, toCheckPeerList);
        tempTime = time(NULL) - tempTime;
        printf("Finished status check and save in (sec): %lu\n", tempTime);
    }
}

void Crawler::extractToCheckList(std::list<bdNodeId> peers) {
    for (auto & peer : peers) {
        auto *a_data = (uint32_t *) peer.data;
        if (a_data[0] >= regionStart && a_data[0] < regionEnd)
            toCheckPeerList.push_back(peer);
    }
    readyToCheck = true;
}

void Crawler::setRegions(int start, int end) {
    this->regionStart = start;
    this->regionEnd = end;
}

void Crawler::setStage(bool stage) {
    currentStage = stage;
}

void Crawler::setPort(uint16_t newPort) {
    port = newPort;
}

std::list<bdNodeId> Crawler::getToCheckList() {
    std::list<bdNodeId> res;
    std::list<bdNodeId> newOne;
    for (auto & it : toCheckPeerList) {
        if (!bdStdIsZeroNodeId(&it)) {
            auto *a_data = (uint32_t *) it.data;
            if (a_data[0] < regionStart && a_data[0] >= regionEnd)
                res.push_back(it);
            else
                newOne.push_back(it);
        }
    }
    toCheckPeerList.clear();
    toCheckPeerList.merge(newOne);
    return res;
}

void Crawler::writeLogs() {
    const char *logName = "dhtlogs";
    if (!dhtHandler) {
        std::cerr << "Problem with dhtHandler, can't write to logs" << std::endl;
        return;
    }
    if (!dhtHandler->getEnabled() || !dhtHandler->getActive()) {
        std::cerr << "dhtHandler is disabled, can't write to logs" << std::endl;
        return;
    }

    std::vector<std::string> logs(dhtHandler->mUdpBitDht->getFoundPeers());
    FILE *tempFile = fopen(logName, "a+");
    bdNodeId tempId = {};
    for (auto & log : logs) {
        if (log.empty())
            continue;
        // TODO: can be done better
        std::string accum;
        for (short i = 0; i < 40; i++) {
            if (log[i] != ' ')
                accum += log[i];
            else
                memcpy(tempId.data, accum.c_str(), sizeof(tempId.data));
        }
        toCheckPeerList.insert(toCheckPeerList.end(), tempId);

        if (!log.empty())
            if (fprintf(tempFile, "%s\n", log.c_str()) < 0)
                std::cerr << "While whiting to dhtlogs accrued an err=%d: %s\n", errno, strerror(errno);
    }
    fclose(tempFile);
}
