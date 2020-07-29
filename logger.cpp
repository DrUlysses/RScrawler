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
#include <algorithm>
#include <bdstddht.h>

#define TICK_PAUSE	5
#define USED_IDS_FILENAME "my_ids"

std::map<bdNodeId, bdFilteredPeer> Logger::discoveredPeers = {};

Logger::Logger() : dhtMutex(true) {
    //bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
    //std::map<bdNodeId, bdFilteredPeer> Logger::discoveredPeers = {};
}

Logger::~Logger() noexcept {
    return;
}

void Logger::stop() {
    isAlive = false;
}

void Logger::disable() {
    isActive = false;
    bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
    sortRsPeers();
}

void Logger::enable() {
    isActive = true;
}

/*** Overloaded from iThread ***/
void Logger::run() {
    while (isAlive) {
        if (isActive) {
            bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
            Logger::iteration();

            std::remove(DHT_FILENAME);
            FILE *logsFile = fopen(DHT_FILENAME, "w");
            std::string tempID;
            for (std::map<bdNodeId, bdFilteredPeer>::iterator it = Logger::discoveredPeers.begin(); it != Logger::discoveredPeers.end(); it++) {
                bdStdPrintNodeId(tempID, &it->first, false);
                fprintf(logsFile, "%s %s %u %lu\n", tempID.c_str(), bdnet_inet_ntoa(it->second.mAddr.sin_addr).c_str(),
                        it->second.mFilterFlags, it->second.mLastSeen);
            }
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
        size_t pos;
        short counter = 0;
        for (short i = 0; i < line.length(); i++) {
            if (line[i] != ' ' and i != line.length() - 1)
                accum += line[i];
            else {
                switch (counter) {
                    case 0:
                        if (accum.length() != 40) {
                            accum = "";
                            break;
                        }
                        memcpy(id.data, accum.c_str(), sizeof(id.data));
                        counter++;
                        accum = "";
                        break;
                    case 1:
                        pos = accum.find(':');
                        if (pos != std::string::npos)
                            accum.erase(pos, 6);
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
                    default:
                        accum = "";
                        break;
                }
            }
        }

        char addr_c_str[addr_str.length()];
        memcpy(addr_c_str, addr_str.c_str(), sizeof(addr_str));
        if (bdnet_inet_aton(addr_c_str, &(addr.sin_addr)) && this != NULL) {
            bdFilteredPeer tempPeer{};
            tempPeer.mAddr = addr;
            tempPeer.mFilterFlags = status;
            tempPeer.mLastSeen = timeStamp;

            Logger::discoveredPeers[id] = tempPeer;
        }
    }
    logsFile.close();
}

void Logger::sortRsPeers(std::list<bdId>* /*result*/) {
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
                        bdStdLoadNodeId(&id.id, accum);
                        if (accum.length() != 40) {
                            accum = "";
                            break;
                        }
                        counter++;
                        accum = "";
                        break;
                    case 1:
                        accum.erase(accum.find(':'), 6);
                        addr_str = accum;
                        counter++;
                        accum = "";
                        break;
                    case 2:
                        memcpy(version.data, accum.c_str(), sizeof(version.data));
                        counter++;
                        accum = "";
                        break;
                    default:
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

    //result = new std::list<bdId>[RSPeers.size()]();

    std::ifstream myIDs(USED_IDS_FILENAME);
    if (!myIDs) {
        std::cerr << "Failed to open my IDs file" << std::endl;
        return;
    }
    std::list<std::string> myIDsList;
    while (std::getline(myIDs, line))
        myIDsList.push_back(line);
    myIDs.close();

    FILE *filtered = fopen(FILTERED_FILENAME, "a+");
    std::map<bdId, bdToken>::iterator it;
    for (it = RSPeers.begin(); it != RSPeers.end(); it++) {
        /*if (result != nullptr)
            result->push_back(it->first);*/
        bdStdPrintNodeId(line, &it->first.id, false);
        if (std::find(myIDsList.begin(), myIDsList.end(), line) == myIDsList.end()) {
            fprintf(filtered, "%s %s %s\n", line.c_str(), bdnet_inet_ntoa(it->first.addr.sin_addr).c_str(),
                    it->second.data);
            bdFilteredPeer tempPeer;
            tempPeer.mAddr = it->first.addr;
            Logger::discoveredPeers[it->first.id] = tempPeer;
        }
    }
    fclose(filtered);
    return;
}

std::list<bdNodeId> Logger::getDiscoveredPeers() {
    std::list<bdNodeId> res;
    for (std::map<bdNodeId, bdFilteredPeer>::iterator it = Logger::discoveredPeers.begin(); it != Logger::discoveredPeers.end(); it++)
        res.push_back(it->first);
    return res;
}

