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

void Crawler::initDhtHandler() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler = new BitDhtHandler(peerId, port, appId, bootstrapfile);
    }
}

void Crawler::stopDht() {
    isActive = false;
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->enable(false);
        dhtHandler->shutdown();
    }
}

void Crawler::startDht() {
    isActive = true;
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler->start();
        if (dhtHandler != nullptr)
            dhtHandler->enable(true);
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

bool Crawler::getActive() {
    return isActive && dhtHandler->getEnabled();
}

void Crawler::setCrawlsCount(unsigned int count) {
    crawlsCount = count;
}

void Crawler::run() {
    std::cerr << "ENTERED RUN" << std::endl;
    for (unsigned int i = 0; i < crawlsCount; i++) {
        if (isActive) {
            if (currentStage == 0) {
                bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
                Crawler::iterationFirstStage();
            } else {
                bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
                Crawler::iterationSecondStage();
            }
            writeLogs();
        }
        sleep(TICK_PAUSE);
    }
}

void Crawler::iterationFirstStage() {
    // Send random request to unknown peer
    bdNodeId searchId{};
    bdStdRandomIdFromRegion(&searchId, regionStart, regionEnd);
    while (std::find(toCheckPeerList.begin(), toCheckPeerList.end(), searchId) != toCheckPeerList.end())
        bdStdRandomIdFromRegion(&searchId, regionStart, regionEnd);
    bdId resultId;
    uint32_t status;

    resultId.id = searchId;

    short ticksDone = 0;
    while (!(dhtHandler->SearchResult(&resultId, status) || ticksDone >= PEER_RECONNECT_TICKS)) {
        sleep(TICK_LENGTH);
        ticksDone++;
    }
}

void Crawler::iterationSecondStage() {
    if (!toCheckPeerList.empty()) {
        time_t tempTime = time(NULL);
        bdId resultId;
        uint32_t status;
        for (auto peerId : toCheckPeerList) {
            resultId.id = peerId;

            short ticksDone = 0;
            while (!(dhtHandler->SearchResult(&resultId, status) || ticksDone >= PEER_RECONNECT_TICKS)) {
                sleep(TICK_LENGTH);
                ticksDone++;
            }
        }
        tempTime = time(NULL) - tempTime;
        printf("Finished status check and save in (sec): %lu\n", tempTime);
    } else
        iterationFirstStage();
}

void Crawler::extractToCheckList(std::list<bdNodeId> peers) {
    toCheckPeerList.clear();
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

void Crawler::setBDBoot(std::string path) {
    if (!path.empty())
        bootstrapfile = std::move(path);
}

void Crawler::setPort(uint16_t newPort) {
    port = newPort;
}

std::list<bdNodeId> Crawler::getToCheckList() {
    return toCheckPeerList;
}

void Crawler::writeLogs() {
    bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
    const char *logName = "dhtlogs";
    if (!dhtHandler) {
        std::cerr << "Problem with dhtHandler, can't write to logs" << std::endl;
        return;
    }
    if (!dhtHandler->getEnabled()) {
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
    std::cout << "Successfully written to logs" << std::endl;
}
