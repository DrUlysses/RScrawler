#include <zconf.h>
#include <bdstddht.h>
#include <bootstrap_fn.h>
#include "crawler.h"

#define TICK_PAUSE 480 // in seconds
#define SEARCH_SHOTS_COUNT 5
#define SEARCH_ROUNDS_COUNT 3
#define USED_IDS_FILENAME "my_ids"

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
    isAlive = false;
    bdStackMutex stackMutex(crwlrMutex);
    dhtHandler->shutdown();
}

void Crawler::disable() {
    isActive = false;
}

void Crawler::enable() {
    isActive = true;
}

void Crawler::run() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        dhtHandler = new BitDhtHandler(peerId, port, appId, bootstrapfile);
    }
    while(isAlive) {
        if (isActive) {
            bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
            if (currentStage == 0)
                Crawler::iterationFirstStage();
            else
                Crawler::iterationSecondStage();
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
        std::map<bdNodeId, std::pair<std::string, time_t>> statuses = bdFindPeers(*dhtHandler, toCheckPeerList);
        std::map<bdNodeId, std::pair<std::string, time_t>>::iterator it;
        std::string tempFileName;
        std::string folderPrefix = "statuses_database/";
        bdNodeId tempID;
        for (it = statuses.begin(); it != statuses.end(); it++) {
            tempID = it->first;
            bdStdLoadNodeId(&tempID, tempFileName);
            tempFileName = folderPrefix + tempFileName;
            FILE *idInfo = fopen(tempFileName.c_str(), "a+");
            fprintf(idInfo, "%s %lu\n", it->second.first.c_str(), it->second.second);
            fclose(idInfo);
        }
        tempTime = time(NULL) - tempTime;
        printf("Finished status check and save in (sec): %lu\n", tempTime);
    }
}

void Crawler::extractToCheckList(std::list<bdNodeId> peers) {
    for (std::list<bdNodeId>::iterator it = peers.begin(); it != peers.end(); it++) {
        uint32_t *a_data = (uint32_t *) it->data;
        if (a_data[0] >= regionStart && a_data[0] < regionEnd)
            toCheckPeerList.push_back(*it);
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
    for (std::list<bdNodeId>::iterator it = toCheckPeerList.begin(); it != toCheckPeerList.end(); it++) {
        uint32_t *a_data = (uint32_t *) it->data;
        if (a_data[0] < regionStart && a_data[0] >= regionEnd)
            res.push_back(*it);
        else
            newOne.push_back(*it);
    }
    toCheckPeerList.clear();
    toCheckPeerList.merge(newOne);
    return res;
}
