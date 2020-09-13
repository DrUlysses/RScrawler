#include <iostream>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <bdstddht.h>
#include "libbitdht/src/example/bootstrap_fn.h"
#include "libbitdht/src/rsids.h"
#include "logger.h"
#include "crawler.h"

#define CRAWLERS_COUNT 32
#define CRAWL_DURATION 10 // in seconds
#define CRAWLS_COUNT 2
#define CHECKS_COUNT 3
#define DURATION_BETWEEN_CHECKS 30 // in seconds

void args(char *name) {
    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
}

void firstStage(std::vector<Crawler>& crawlers, Logger& logger);
void secondStage(std::vector<Crawler>& crawlers, Logger& logger);

int main(int argc, char **argv) {
    // Creates cr_count + 1 then destroys one. Why? I don't know (probably it needs to have some copy constructor)
    std::vector<Crawler> crawlers(CRAWLERS_COUNT);

    Logger* logger = new Logger();

    // Find node search
    firstStage(crawlers, *logger);

    std::cerr << "Crawls are finished";
    std::cerr << std::endl;

    // Ping-pong status and version check
    secondStage(crawlers, *logger);

    std::cerr << "Watchers are finished";
    std::cerr << std::endl;

    return 0;
}

void firstStage(std::vector<Crawler>& crawlers, Logger& logger) {
    logger.start();
    // Setting regions
    int regionStart = 0;
    int regionLength = 32 / CRAWLERS_COUNT;
    int regionEnd = regionLength;
    uint16_t port = 6775;
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i].init();
        crawlers[i].setStage(0);
        crawlers[i].setRegions(regionStart, regionEnd);
        crawlers[i].setPort(port++);
        crawlers[i].start();
        regionStart = regionEnd + 1;
        regionEnd = i == CRAWLERS_COUNT - 2 ? 32 : regionEnd + regionLength;
    }
    for (unsigned int i = 0; i < CRAWLS_COUNT; i++) {
        // Waiting for crawlers duty
        sleep(CRAWL_DURATION);
        // Sorting out of region IDs
        std::list<bdNodeId> tempIDStorage = logger.getDiscoveredPeers();
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            tempIDStorage.merge(crawlers[j].getToCheckList());
        tempIDStorage.merge(logger.getDiscoveredPeers());

        crawlers[i].restart();

        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j].extractToCheckList(tempIDStorage);
    }
    // Pause crawling
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i].enable(false);
        crawlers[i].stop();
    }
    logger.disable();
}

void secondStage(std::vector<Crawler>& crawlers, Logger& logger) {
    logger.enable();

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i].setStage(1);
        crawlers[i].enable(true);
    }
    // Sorting out of region IDs
    for (unsigned int i = 0; i < CHECKS_COUNT; i++) {
        logger.sortRsPeers();
        sleep(DURATION_BETWEEN_CHECKS);
        std::list<bdNodeId> tempIDStorage;
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            tempIDStorage.merge(crawlers[j].getToCheckList());
        tempIDStorage.merge(logger.getDiscoveredPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j].extractToCheckList(tempIDStorage);
    }

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        bdStackMutex stackMutex(crawlers[i].mMutex);
        crawlers[i].stop();
    }

    logger.stop();
}