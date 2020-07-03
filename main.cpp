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
#define CRAWL_DURATION 3600// in seconds

void args(char *name) {
    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
}

int main(int argc, char **argv) {

    Logger *logger = new Logger();
    logger->start();

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

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i].stop();

    logger->stop();

    logger.

    std::cerr << "Crawls finished";
    std::cerr << std::endl;

    return 1;
}
