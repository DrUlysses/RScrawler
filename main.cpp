#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <inttypes.h>
#include <bdstddht.h>
#include "libbitdht/src/rsids.h"
#include "logger.h"
#include "crawler.h"

#define CRAWLERS_COUNT 32 // <= 32
#define CRAWL_DURATION 30 // in seconds
#define CRAWLS_COUNT 2
#define DURATION_BETWEEN_CHECKS 30 // in seconds >= 80
#define CHECKS_COUNT 3
#define DURATION_BETWEEN_CRAWLS 21600 // 6 hours (in seconds) 21600
#define LOG_FILENAME "dhtlogs"
#define RS_PEERS_FILENAME "rspeers"

//void args(char *name) {
//    std::cerr << std::endl << "Dht Single Shot Searcher" << std::endl;
//    std::cerr << "Usage:" << std::endl;
//    std::cerr << "\t" << name << " -p <peerId> " << std::endl << std::endl;
//    std::cerr << "NB: The PeerId is Required to Run" << std::endl << std::endl;
//}

void firstStage(std::vector<Crawler*>& crawlers, Logger& logger);
void secondStage(std::vector<Crawler*>& crawlers, Logger& logger);

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

std::string copyBDBoot(unsigned int crwlrNumber) {
    const std::filesystem::path src = "../libbitdht/src/example/bdboot.txt";
    std::string dstPath = "bdboots/bdboot_crwlr_" + std::to_string(crwlrNumber) + ".txt";
    std::filesystem::create_directory("bdboots");
    const std::filesystem::path dst = dstPath;

    std::filesystem::copy(src, dst, std::filesystem::copy_options::overwrite_existing);

    return dstPath;
}

int main(int argc, char **argv) {

    std::vector<Crawler*> crawlers(CRAWLERS_COUNT);
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i] = new Crawler();

    auto* logger = new Logger();
    logger->disable();
    logger->start();
    // Crawlers initialization
    int regionStart = 0;
    int regionLength = 32 / CRAWLERS_COUNT;
    int regionEnd = regionLength;
    uint16_t port = 6775;
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i]->init();
        crawlers[i]->setBDBoot(copyBDBoot(i));
        crawlers[i]->setStage(0);
        crawlers[i]->setRegions(regionStart, regionEnd);
        crawlers[i]->setPort(port++);
        crawlers[i]->setCrawlsCount(CRAWLS_COUNT);
        crawlers[i]->initDhtHandler();
        crawlers[i]->start();
        regionStart = regionEnd + 1;
        regionEnd = i == CRAWLERS_COUNT - 2 ? 32 : regionEnd + regionLength;
    }
    // One week
    for (short i = 0; i < 7; i++) {
        // Whole day loop
        for (unsigned int  j = 0; j < (24 * 60 * 60) / DURATION_BETWEEN_CRAWLS; j++) {
            // Find node search
            firstStage(crawlers, *logger);

            std::cerr << "Crawls are finished" << std::endl;

            // Ping-pong status and version check
            secondStage(crawlers, *logger);

            std::cerr << "Watchers are finished" << std::endl;

            // Run analyzer
            exec("../analyzer/run.sh");

            std::cerr << "Waiting for the next crawl" << std::endl;

            // Wait for the next crawl
            sleep(DURATION_BETWEEN_CRAWLS);

            // Delete old log files
            remove(LOG_FILENAME);
            remove(RS_PEERS_FILENAME);
        }
    }

    // Cleanup
    for (unsigned i = 0; i < CRAWLERS_COUNT; i++)
        delete crawlers[i];
    delete logger;

    return 0;
}

void firstStage(std::vector<Crawler*>& crawlers, Logger& logger) {
    // Launch crawlers and logger
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i]->startDht();
    logger.enable();

    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CRAWLS_COUNT; i++) {
        // Waiting for crawlers duty
        sleep(CRAWL_DURATION);
        // Get the previously found peers
        tempIDStorage.merge(Logger::getDiscoveredPeers());
        // Sorting out of region IDs
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j]->enable(false);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            std::cerr << "Waiting for crawler " << std::to_string(j) << std::endl;
            crawlers[j]->enableDht(false);
            // Get the found nodes lists
            tempIDStorage.merge(crawlers[j]->getToCheckList());
            // Restart nodes, not to hold the handler for too long
            crawlers[j]->restart();
            // Inject new peers to check and activate crawlers
            crawlers[j]->extractToCheckList(tempIDStorage);
            crawlers[j]->enableDht(true);
            crawlers[j]->enable(true);
        }
    }
    // Pause crawling and logging
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i]->enable(false);
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        std::cerr << "Waiting for crawler " << std::to_string(i) << std::endl;
        crawlers[i]->enableDht(false);
    }
    logger.disable();
}

void secondStage(std::vector<Crawler*>& crawlers, Logger& logger) {
    // Enable logger and crawlers, set second stage
    logger.enable();
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i]->setStage(1);
        crawlers[i]->enableDht(true);
        crawlers[i]->enable(true);
    }
    // Sorting out of region IDs (same as in the first stage)
    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CHECKS_COUNT; i++) {
        sleep(DURATION_BETWEEN_CHECKS);
        logger.sortRsPeers();
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j]->enable(false);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            std::cerr << "Waiting for crawler " << std::to_string(j) << std::endl;
            crawlers[j]->enableDht(false);
            tempIDStorage.merge(crawlers[j]->getToCheckList());
        }
        tempIDStorage.merge(Logger::getDiscoveredPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            crawlers[j]->extractToCheckList(tempIDStorage);
            crawlers[j]->restart();
            crawlers[j]->enableDht(true);
            crawlers[j]->enable(true);
        }
    }
    // Stop the crawlers and logger
    for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
        crawlers[j]->enable(false);
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        std::cerr << "Waiting for crawler " << std::to_string(i) << std::endl;
        crawlers[i]->enableDht(false);
        crawlers[i]->stopDht();
    }
    logger.disable();
}