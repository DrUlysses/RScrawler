#include <zconf.h>
#include <netinet/in.h>
#include <bdiface.h>
#include <map>
#include "logger.h"
#include <bdpeer.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cstring>

Logger::Logger() : dhtMutex(true) {
    bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
}

Logger::~Logger() noexcept {
    return;
}

void Logger::stop() {
    isAlive = false;
}

#define TICK_PAUSE	5

/*** Overloaded from iThread ***/

void Logger::run() {
    while (isAlive) {
        {
            bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
            Logger::iteration();

            std::remove(DHT_FILENAME);
            FILE *logsFile = fopen(DHT_FILENAME, "w");
            std::map<bdNodeId, bdFilteredPeer>::iterator it;
            for (it = discoveredPeers.begin(); it != discoveredPeers.end(); it++)
                fprintf(logsFile, "%s %s %u %lu\n", it->first.data, bdnet_inet_ntoa(it->second.mAddr.sin_addr).c_str(),
                        it->second.mFilterFlags, it->second.mLastSeen);
            fclose(logsFile);
        }
        sleep(TICK_PAUSE);
    }
}

void Logger::iteration() {
    std::string line;
    std::string addr_str;
    uint32_t status;
    struct sockaddr_in addr;
    bdNodeId id;
    time_t timeStamp;

    std::ifstream logsFile(LOG_FILENAME);
    if (!logsFile) {
        std::cerr << "Failed to open dht logs file" << std::endl;
        return;
    }
    while (std::getline(logsFile, line)) {
        // I HATE C++ STRINGS
        std::string accum;
        short counter = 0;
        for (short i = 0; i < line.length(); i++) {
            if (line[i] != ' ' and i != line.length() - 1)
                accum += line[i];
            else {
                switch (counter) {
                    case 0:
                        memcpy(id.data, accum.c_str(), sizeof(id.data));
                        counter++;
                        accum = "";
                        break;
                    case 1:
                        accum.erase(accum.find(':'), 8);
                        addr_str = accum;
                        counter++;
                        accum = "";
                        break;
                    case 2:
                        status = (uint32_t) std::stoul(accum);
                        counter++;
                        accum = "";
                        break;
                    case 3:
                        accum += line[i];
                        timeStamp = (time_t) std::stoul(accum);
                        counter++;
                        accum = "";
                        break;
                }
            }
        }

        char addr_c_str[addr_str.length()];
        memcpy(addr_c_str, addr_str.c_str(), sizeof(addr_str));
        if (bdnet_inet_aton(addr_c_str, &(addr.sin_addr)) && this != NULL) {
            bdFilteredPeer tempPeer;
            tempPeer.mAddr = addr;
            tempPeer.mFilterFlags = status;
            tempPeer.mLastSeen = timeStamp;

            discoveredPeers[id] = tempPeer;
        }
    }
    logsFile.close();
}

void Logger::sortRsPeers() {
    std::string line;
    std::string addr_str;
    bdId id;
    bdToken version;
    std::map<bdId, bdToken> RSPeers;

    std::ifstream logsFile(RS_PEERS_FILENAME);
    if (!logsFile) {
        std::cerr << "Failed to open RS peers list file" << std::endl;
        return;
    }
    while (std::getline(logsFile, line)) {
        // I HATE C++ STRINGS
        std::string accum;
        short counter = 0;
        for (short i = 0; i < line.length(); i++) {
            if (line[i] != ' ' and i != line.length() - 1)
                accum += line[i];
            else {
                switch (counter) {
                    case 0:
                        memcpy(id.id.data, accum.c_str(), sizeof(id.id));
                        counter++;
                        accum = "";
                        break;
                    case 1:
                        accum.erase(accum.find(':'), 8);
                        addr_str = accum;
                        counter++;
                        accum = "";
                        break;
                    case 2:
                        memcpy(version.data, accum.c_str(), sizeof(version.data));
                        counter++;
                        accum = "";
                        break;
                }
            }
        }

        char addr_c_str[addr_str.length()];
        memcpy(addr_c_str, addr_str.c_str(), sizeof(addr_str));
        if (bdnet_inet_aton(addr_c_str, &(id.addr.sin_addr)))
            RSPeers[id] = version;

    }
    logsFile.close();
    FILE *filtered = fopen(FILTERED_FILENAME, "w");
    std::map<bdId, bdToken>::iterator it;
    for (it = RSPeers.begin(); it != RSPeers.end(); it++)
        fprintf(filtered, "%s %s %s\n", it->first.id.data, bdnet_inet_ntoa(it->first.addr.sin_addr).c_str(), it->second.data);
    fclose(filtered);
}

