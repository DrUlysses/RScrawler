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
#define CRAWL_DURATION 20 // in seconds
#define CRAWLS_COUNT 2
#define CHECKS_COUNT 3
#define DURATION_BETWEEN_CHECKS 30 // in seconds
#define DURATION_BETWEEN_CRAWLS 21600 // 6 hours (in seconds)
#define LOG_FILENAME "dhtlogs"

void args(char *name) {
    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
}

void firstStage(std::vector<Crawler>& crawlers, Logger& logger);
void secondStage(std::vector<Crawler>& crawlers, Logger& logger);

int main(int argc, char **argv) {

    // Generate up-to-date bdboot.txt
//    system("cat libbitdht/src/bitdht/bdboot.txt | ./libbitdht/src/bitdht/bdboot_generate.sh | tee /libbitdht/src/bitdht/tmp/bdboot_generated.txt\n"
//           "cat libbitdht/src/bitdht//tmp/bdboot_generated.txt | sort -u > libbitdht/src/bitdht/bdboot.txt");

    std::vector<Crawler> crawlers(CRAWLERS_COUNT);

    Logger* logger = new Logger();

    // One week
    for (short i = 0; i < 7; i++) {
        // Whole day loop
        for (short  j = 0; j < (24 * 60 * 60) / DURATION_BETWEEN_CRAWLS; j++)
        // Find node search
        firstStage(crawlers, *logger);

        std::cerr << "Crawls are finished" << std::endl;

        // Ping-pong status and version check
        secondStage(crawlers, *logger);

        std::cerr << "Watchers are finished" << std::endl;

        std::cerr << "Waiting for the next crawl" << std::endl;

        // Run analyzer
        system("../analyzer/run.sh");

        // Wait for the next crawl
        sleep(DURATION_BETWEEN_CRAWLS);

        // Delete old log file
        remove(LOG_FILENAME);
    }

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
        crawlers[i].setCrawlsCount(CRAWLS_COUNT);
        crawlers[i].start();
        regionStart = regionEnd + 1;
        regionEnd = i == CRAWLERS_COUNT - 2 ? 32 : regionEnd + regionLength;
    }
    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CRAWLS_COUNT; i++) {
        // Waiting for crawlers duty
        sleep(CRAWL_DURATION);
        // Sorting out of region IDs
        tempIDStorage.merge(logger.getDiscoveredPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            crawlers[i].setActive(false);
            tempIDStorage.merge(crawlers[j].getToCheckList());
        }
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
        crawlers[i].start();
        crawlers[i].enable(true);
    }
    // Sorting out of region IDs
    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CHECKS_COUNT; i++) {
        logger.sortRsPeers();
        sleep(DURATION_BETWEEN_CHECKS);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            tempIDStorage.merge(crawlers[j].getToCheckList());
        tempIDStorage.merge(logger.getDiscoveredPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j].extractToCheckList(tempIDStorage);
        crawlers[i].restart();
    }

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        bdStackMutex stackMutex(crawlers[i].mMutex);
        crawlers[i].stop();
    }

    logger.stop();
}