#include <iostream>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <bdstddht.h>
#include "libbitdht/src/example/bootstrap_fn.h"
#include "libbitdht/src/rsids.h"
#include "logger.h"
#include "crawler.h"

#define CRAWLERS_COUNT 3
#define CRAWL_DURATION 300 // in seconds

void args(char *name) {
    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
}

void firstStage();

int main(int argc, char **argv) {

    firstStage();

    std::cerr << "Crawls are finished";
    std::cerr << std::endl;

    return 0;
}

void firstStage() {
    Logger *logger = new Logger();
    logger->start();

    // Creates cr_count + 1 then destroys one. Why? I don't know (probably it needs to have some copy constructor)
    std::vector<Crawler> crawlers(CRAWLERS_COUNT);
    std::fill(crawlers.begin(), crawlers.end(), Crawler());

    int regionStart = 0;
    int regionLength = 32 / CRAWLERS_COUNT;
    int regionEnd = regionLength;
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i].setRegions(regionStart, regionEnd);
        crawlers[i].start();
        regionStart = regionEnd + 1;
        regionEnd = i == CRAWLERS_COUNT - 2 ? 32 : regionEnd + regionLength;
    }

    sleep(CRAWL_DURATION);

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        bdStackMutex stackMutex(crawlers[i].mMutex);
        crawlers[i].stop();
    }

    {
        bdStackMutex stackMutex(logger->mMutex);
        logger->stop();
    }

    logger->sortRsPeers();
}
