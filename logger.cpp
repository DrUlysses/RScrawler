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

std::map<bdNodeId, bdFoundPeer> Logger::discoveredPeers = {};

Logger::Logger() : dhtMutex(true) {
    //bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
    //std::map<bdNodeId, bdFilteredPeer> Logger::discoveredPeers = {};
}

Logger::~Logger() noexcept {
    std::cerr << "Logger object destroyed" << std::endl;
}

void Logger::stop() {
    isAlive = false;
}

void Logger::enable(bool state) {
    isActive = state;
}

/*** Overloaded from iThread ***/
void Logger::run() {
    while (isAlive) {
        if (isActive) {
            bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
            Logger::iteration();
        }
        sleep(TICK_PAUSE);
    }
}

void Logger::iteration() {
    std::string line;
    std::string addr_str;
    std::string version;
    struct sockaddr_in addr;
    bdNodeId id;
    time_t timeStamp;

    std::ifstream logsFile(LOG_FILENAME);
    if (!logsFile) {
        std::cerr << "Failed to open dht logs file" << std::endl;
        return;
    }
    while (std::getline(logsFile, line)) {
        version = "";
        addr_str = "";
        addr = {};
        id = {};
        timeStamp = time(NULL);
        if (line.empty())
            continue;
        // I HATE C++ STRINGS
        std::string accum;
        size_t pos;
        short counter = 0;
        for (short i = 0; i < line.length() - 1; i++) {
            if (counter == -10)
                break;
            if (line[i] != ' ')
                accum += line[i];
            else {
                switch (counter) {
                    case 0:
                        if (accum.length() != 40) {
                            accum = "";
                            counter = -10;
                            break;
                        }
                        memcpy(id.data, accum.c_str(), sizeof(id.data));
                        counter++;
                        accum = "";
                        break;
                    case 1:
                        if (std::count(accum.begin(), accum.end(), '.') != 3 || std::count(accum.begin(), accum.end(), '.') != 1)
                            counter = -10;
                        else {
                            pos = accum.find(':');
                            if (pos != std::string::npos)
                                accum.erase(pos, 6);
                            addr_str = accum;
                            counter++;
                        }
                        accum = "";
                        break;
                    case 2:
                        accum += line[i];
                        version = accum;
                        counter++;
                        accum = "";
                        break;
                    case 3:
                        // Won't work after some time (kinda current time dependent)
                        if (accum.length() >= 10) {
                            timeStamp = (time_t) std::stoul(accum);
                            counter++;
                            accum = "";
                        } else {
                            timeStamp = time(NULL);
                            break;
                        }
                        break;
                    default:
                        accum = "";
                        break;
                }
            }
        }
        // Check if read data was uncorrupted
        if (counter > 0) {
            char addr_c_str[addr_str.length()];
            memcpy(addr_c_str, addr_str.c_str(), sizeof(addr_str));
            if (bdnet_inet_aton(addr_c_str, &(addr.sin_addr)) && this != NULL) {
                bdFoundPeer tempPeer{};
                tempPeer.mAddr = addr;
                tempPeer.mVersion = version;
                tempPeer.mLastSeen = timeStamp;

                Logger::discoveredPeers[id] = tempPeer;
            }
        }
    }
    logsFile.close();
}

std::list<bdNodeId> Logger::getDiscoveredRSPeers() {
    std::list<bdNodeId> res;
    for (auto & discoveredPeer : Logger::discoveredPeers) {
        if (discoveredPeer.second.mVersion == "BD02RS51")
            res.push_back(discoveredPeer.first);
    }
    return res;
}

std::list<bdNodeId> Logger::getDiscoveredPeers() {
    std::list<bdNodeId> res;
    for (auto & discoveredPeer : Logger::discoveredPeers)
        res.push_back(discoveredPeer.first);
    return res;
}

