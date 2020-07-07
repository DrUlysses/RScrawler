#include <zconf.h>
#include <bdstddht.h>
#include <bootstrap_fn.h>
#include "crawler.h"

#define TICK_PAUSE 480 // in seconds
#define SEARCH_SHOTS_COUNT 5
#define SEARCH_ROUNDS_COUNT 3
#define USED_IDS_FILENAME "my_ids"

Crawler::Crawler() : crwlrMutex(true) {
    //bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
    crwlrMutex.lock();
    std::cerr << "Crawler object created\n";
    //peerId = RsPeerId::random().toStdString();
    bdStdRandomNodeId(&peerId);
    std::cerr << "Starting with ownID: ";
    bdStdPrintNodeId(std::cerr, &peerId);
    std::cerr << std::endl;
    FILE *myids = fopen(USED_IDS_FILENAME, "a+");
    std::string stringID;
    bdStdPrintNodeId(stringID, &peerId, false);
    fprintf(myids, "%s\n", stringID.c_str());
    fclose(myids);
    crwlrMutex.unlock();
}

Crawler::~Crawler() noexcept {
    std::cerr << "Crawler object destroyed, id: ";
    bdStdPrintNodeId(std::cerr, &peerId);
    std::cerr << std::endl;
    return;
}

void Crawler::stop() {
    bdStackMutex stackMutex(crwlrMutex);
    dhtHandler->shutdown();
    isAlive = false;
}

void Crawler::run() {

    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        BitDhtHandler dht(&peerId, port, appId, bootstrapfile);
        dhtHandler = &dht;
    }
    while(isAlive) {
        {
            bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
            Crawler::iteration();
        }

        sleep(TICK_PAUSE);
    }
}

void Crawler::iteration() {
    // Start random requests from separate threads
    bdSingleSourceFindPeer(*dhtHandler, peerId, port, SEARCH_SHOTS_COUNT, SEARCH_ROUNDS_COUNT, regionStart, regionEnd);
}

void Crawler::setRegions(int start, int end) {
    this->regionStart = start;
    this->regionEnd = end;
}
