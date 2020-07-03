#include <zconf.h>
#include <bdstddht.h>
#include <bootstrap_fn.h>
#include "crawler.h"

#define TICK_PAUSE 480
#define SEARCH_SHOTS_COUNT 10
#define SEARCH_ROUNDS_COUNT 3

Crawler::Crawler() : crwlrMutex(true) {
    bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/

    std::cerr << "Crawler object created\n";
    //peerId = RsPeerId::random().toStdString();
    bdStdRandomNodeId(&peerId);
    std::cerr << "Starting with ownID: ";
    bdStdPrintNodeId(std::cerr, &peerId);
    std::cerr << std::endl;
}

Crawler::~Crawler() noexcept {
    std::cerr << "Crawler object destroyed, id: ";
    bdStdPrintNodeId(std::cerr, &peerId);
    std::cerr << std::endl;
    return;
}

void Crawler::run() {
    {
        bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/
        BitDhtHandler dht(&peerId, port, appId, bootstrapfile);
        dhtHandler = &dht;
    }
    while(1) {
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
