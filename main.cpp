#include <iostream>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <bdstddht.h>
#include "libbitdht/src/example/bootstrap_fn.h"
#include "libbitdht/src/rsids.h"
#include "logger.h"
#include "crawler.h"

#define CRAWLERS_COUNT 1
#define CRAWL_DURATION 30 // in seconds
#define CRAWLS_COUNT 2
#define DURATION_BETWEEN_CHECKS 30 // in seconds
#define CHECKS_COUNT 3
#define DURATION_BETWEEN_CRAWLS 21600 // 6 hours (in seconds) 21600
#define LOG_FILENAME "dhtlogs"
#define RS_PEERS_FILENAME "rspeers"

/*void args(char *name) {
    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
}*/

void firstStage(std::vector<Crawler>& crawlers, Logger& logger);
void secondStage(std::vector<Crawler>& crawlers, Logger& logger);

void exec(const char* cmd) {
    char buffer[16];
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
        std::cerr << "popen() failed!" << std::endl;
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr)
            std::cerr << buffer;
    } catch (...) {
        pclose(pipe);
        std::cerr << "problem with popen()" << std::endl;
    }
    pclose(pipe);
    std::cerr << "Analyzer is closed" << std::endl;
}

int main(int argc, char **argv) {

    std::vector<Crawler> crawlers(CRAWLERS_COUNT);

    Logger* logger = new Logger();

    // Setting crawlers
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
        crawlers[i].initDhtHandler();
        regionStart = regionEnd + 1;
        regionEnd = i == CRAWLERS_COUNT - 2 ? 32 : regionEnd + regionLength;
    }
    // One week
    for (short i = 0; i < 7; i++) {
        // Whole day loop
        for (short  j = 0; j < (24 * 60 * 60) / DURATION_BETWEEN_CRAWLS; j++) {
            // Find node search
            firstStage(crawlers, *logger);

            std::cerr << "Crawls are finished" << std::endl;

            // Ping-pong status and version check
            secondStage(crawlers, *logger);

            std::cerr << "Watchers are finished" << std::endl;

            std::cerr << "Waiting for the next crawl" << std::endl;

            // Run analyzer
            exec("../analyzer/run.sh");

            // Wait for the next crawl
            sleep(DURATION_BETWEEN_CRAWLS);

            // Delete old log files
            remove(LOG_FILENAME);
            remove(RS_PEERS_FILENAME);
        }
    }

    return 0;
}

void firstStage(std::vector<Crawler>& crawlers, Logger& logger) {
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i].start();
    logger.start();

    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CRAWLS_COUNT; i++) {
        // Waiting for crawlers duty
        sleep(CRAWL_DURATION);
        // Sorting out of region IDs
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            crawlers[i].setActive(false);
            tempIDStorage.merge(crawlers[j].getToCheckList());
        }
        crawlers[i].writeLogs();
        tempIDStorage.merge(logger.getDiscoveredPeers());

        crawlers[i].restart();

        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            crawlers[j].extractToCheckList(tempIDStorage);
            crawlers[i].setActive(true);
        }
    }
    // Pause crawling
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i].enable(false);
        while (crawlers[i].getActive()) {
            std::cerr << "Waiting for crawler " << std::to_string(i) << std::endl;
            sleep(1);
        }
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
    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CHECKS_COUNT; i++) {
        crawlers[i].writeLogs();
        crawlers[i].restart();
        logger.sortRsPeers();
        sleep(DURATION_BETWEEN_CHECKS);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            tempIDStorage.merge(crawlers[j].getToCheckList());
        tempIDStorage.merge(logger.getDiscoveredPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j].extractToCheckList(tempIDStorage);
    }

    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        bdStackMutex stackMutex(crawlers[i].mMutex);
        crawlers[i].enable(false);
        while (crawlers[i].getActive()) {
            std::cerr << "Waiting for crawler " << std::to_string(i) << std::endl;
            sleep(1);
        }
        crawlers[i].stop();
    }

    logger.stop();
}