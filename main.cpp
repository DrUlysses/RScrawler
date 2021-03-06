#include <iostream>
#include <experimental/filesystem>
#include <stdio.h>
#include <inttypes.h>
#include <bdstddht.h>
#include "libbitdht/src/rsids.h"
#include "logger.h"
#include "crawler.h"

#define CRAWLERS_COUNT 256 // <= 256 More crawlers == more RAM usage, more time to stop them, be careful
#define CRAWL_DURATION 15 // in seconds
#define CRAWLS_COUNT 4
#define DURATION_BETWEEN_CHECKS 80 // in seconds >= 80
#define CHECKS_COUNT 2
#define DURATION_BETWEEN_CRAWLS 10800 // 6 hours = 21600 seconds
#define LOG_FILENAME "dhtlogs"
#define OWN_IDS_FILENAME "my_ids"

void args(char *name) {
    std::cerr << std::endl << "RetroShare crawler" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "\t" << name << " -f <0 or 1> (first run?) " << std::endl << std::endl;
    std::cerr << "NB: The -f Flag set is Required to Run" << std::endl << std::endl;
}

void firstStage(std::vector<Crawler*>& crawlers, Logger& logger);
void secondStage(std::vector<Crawler*>& crawlers, Logger& logger);

void exec(const char* cmd) {
    char buffer[8];
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

std::string copyBDBoot(unsigned int crwlrNumber, bool genNew) {
    std::string dstPath = "bdboots/bdboot_crwlr_" + std::to_string(crwlrNumber) + ".txt";
    if (genNew) {
        const std::experimental::filesystem::path src = "../libbitdht/src/example/bdboot.txt";
        std::experimental::filesystem::create_directory("bdboots");
        const std::experimental::filesystem::path dst = dstPath;

        std::experimental::filesystem::copy(src, dst, std::experimental::filesystem::copy_options::overwrite_existing);
    }
    return dstPath;
}

void createBackup(std::string fileName, int dublicateNumber) {
    std::string dstPath = "backups/" + fileName + std::to_string(dublicateNumber) + ".log";
    const std::experimental::filesystem::path src = fileName;
    std::experimental::filesystem::create_directory("backups");
    const std::experimental::filesystem::path dst = dstPath;
    std::experimental::filesystem::copy(src, dst, std::experimental::filesystem::copy_options::overwrite_existing);
}

int main(int argc, char **argv) {
    int c;
    bool isFlagSet = false;
    bool isFirstRun = true;
    while((c = getopt(argc, argv,"f:")) != -1) {
        switch (c) {
            case 'f':
                isFirstRun = optarg;
                isFlagSet = true;
                break;
            default:
                args(argv[0]);
                return 1;
                break;
        }
    }

    if (!isFlagSet) {
        args(argv[0]);
        return 1;
    }

    // Create objects
    std::vector<Crawler*> crawlers(CRAWLERS_COUNT);

    auto* logger = new Logger();
    logger->enable(false);
    logger->start();
    // Crawlers initialization
    int regionStart = 0;
    // Round the regionLength
    int regionLength = (256 + (CRAWLERS_COUNT / 2)) / CRAWLERS_COUNT;
    // One week
    for (short i = 0; i < 7; i++) {
        // Whole day loop
        for (unsigned int  j = 0; j < (24 * 60 * 60) / DURATION_BETWEEN_CRAWLS; j++) {
            // Time
            time_t startFirstStageTime = time(NULL);

            for (unsigned int k = 0; k < CRAWLERS_COUNT; k++)
                crawlers[k] = new Crawler();
            int regionEnd = regionLength;
            uint16_t port = 6775;
            for (unsigned int k = 0; k < CRAWLERS_COUNT; k++) {
                crawlers[k]->init();
                if (isFirstRun && j == 0)
                    crawlers[k]->setBDBoot(copyBDBoot(k, true));
                else
                    crawlers[k]->setBDBoot(copyBDBoot(k, false));
                crawlers[k]->setStage(0);
                crawlers[k]->setRegions(regionStart, regionEnd);
                crawlers[k]->setPort(port++);
                crawlers[k]->setCrawlsCount(CRAWLS_COUNT);
                crawlers[k]->initDhtHandler();
                crawlers[k]->start();
                regionStart = regionEnd + 1;
                regionEnd = k == CRAWLERS_COUNT - 2 ? 256 : regionEnd + regionLength;
            }

            // Find node search
            firstStage(crawlers, *logger);

            std::cerr << "Crawls are finished in ";
            // Time
            time_t startSecondStageTime = time(NULL);
            std::cerr << startSecondStageTime - startFirstStageTime << " seconds" << std::endl;

            // Ping-pong status and version check
            secondStage(crawlers, *logger);

            std::cerr << "Watchers are finished in ";
            // Time
            time_t endSecondStageTime = time(NULL);
            std::cerr << endSecondStageTime - startSecondStageTime << " seconds" << std::endl;

            time_t nextCrawlStart = startFirstStageTime + DURATION_BETWEEN_CRAWLS;

            // Create backup
            createBackup(LOG_FILENAME, startFirstStageTime);
            createBackup(OWN_IDS_FILENAME, startFirstStageTime);

            // Run analyzer
            exec("../analyzer/run.sh");

            // Time
            time_t endAnalyzerTime = time(NULL);

            std::cerr << "Analyzer done in " << endAnalyzerTime - endSecondStageTime << " seconds" << std::endl;
            std::cerr << "Full crawl cycle done in " << endSecondStageTime - startFirstStageTime << " seconds" << std::endl;

            // Cleanup
            for (unsigned int k = 0; k < CRAWLERS_COUNT; k++) {
                std::cerr << "Deleting crawler " << k << std::endl;
                crawlers[k]->kill();
                crawlers[k]->join();
                delete crawlers[k];
            }

            // Delete old log files
            remove(LOG_FILENAME);
            const std::experimental::filesystem::path src = "new_rs_peers";
            const std::experimental::filesystem::path dst = LOG_FILENAME;
            std::experimental::filesystem::rename(src, dst);
            remove(OWN_IDS_FILENAME);

            // Wait for the next crawl
            nextCrawlStart -= time(NULL);

            std::cerr << "Waiting for the next crawl (" << nextCrawlStart / 60 << " minutes)" << std::endl;
            sleep(nextCrawlStart);
        }
    }

    // Cleanup
    delete logger;

    return 0;
}

void firstStage(std::vector<Crawler*>& crawlers, Logger& logger) {
    // Launch crawlers and logger
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++)
        crawlers[i]->startDht();
    logger.enable(true);

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
        }
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            // Get the found nodes lists
            tempIDStorage.merge(crawlers[j]->getToCheckList());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            // Restart nodes, not to hold the handler for too long
//            crawlers[j]->restart();
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
    logger.enable(false);
}

void secondStage(std::vector<Crawler*>& crawlers, Logger& logger) {
    // Enable logger and crawlers, set second stage
    logger.enable(true);
    for (unsigned int i = 0; i < CRAWLERS_COUNT; i++) {
        crawlers[i]->setStage(1);
        crawlers[i]->enableDht(true);
        crawlers[i]->enable(true);
    }
    // Sorting out of region IDs (same as in the first stage)
    std::list<bdNodeId> tempIDStorage;
    for (unsigned int i = 0; i < CHECKS_COUNT; i++) {
        sleep(DURATION_BETWEEN_CHECKS);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j]->enable(false);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
            std::cerr << "Waiting for crawler " << std::to_string(j) << std::endl;
            crawlers[j]->enableDht(false);
            tempIDStorage.merge(crawlers[j]->getToCheckList());
        }
        tempIDStorage.merge(logger.getDiscoveredRSPeers());
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++)
            crawlers[j]->extractToCheckList(tempIDStorage);
        for (unsigned int j = 0; j < CRAWLERS_COUNT; j++) {
//            crawlers[j]->restart();
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
    logger.enable(false);
}