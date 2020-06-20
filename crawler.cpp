#include <zconf.h>
#include <bdstddht.h>
#include <bootstrap_fn.h>
#include "crawler.h"

#define TICK_PAUSE 480
#define SEARCH_SHOTS_COUNT 5
#define SEARCH_ROUNDS_COUNT 3

Crawler::Crawler() : crwlrMutex(true) {
    bdStackMutex stack(crwlrMutex); /********** MUTEX LOCKED *************/

    std::cerr << "bssdht: starting up";
    std::cerr << std::endl;
    //peerId = RsPeerId::random().toStdString();
    bdStdRandomNodeId(&peerId);

    std::cerr << "bssdht: finished";
    std::cerr << std::endl;
}

Crawler::~Crawler() noexcept {
    return;
}

void Crawler::run() {
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
    bdSingleSourceFindPeer(bootstrapfile, peerId, ip, port, SEARCH_SHOTS_COUNT, SEARCH_ROUNDS_COUNT);
}