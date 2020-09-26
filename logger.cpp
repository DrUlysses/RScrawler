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

void Logger::disable() {
    isActive = false;
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
        timeStamp = 0;
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
                        pos = accum.find(':');
                        if (pos != std::string::npos)
                            accum.erase(pos, 6);
                        addr_str = accum;
                        counter++;
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
                            accum = "";
                            counter = -10;
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

void Logger::sortRsPeers(std::list<bdId>* /*result*/) {
    std::string line;
    std::string addr_str;
    bdId id;
    bdToken version;
    std::map<bdId, std::string> RSPeers;
    std::string timeStamp;

    {
        bdStackMutex stack(dhtMutex); /********** MUTEX LOCKED *************/
        std::ifstream myIDs(USED_IDS_FILENAME);
        if (!myIDs) {
            std::cerr << "Failed to open my IDs file" << std::endl;
            return;
        }
        std::list<std::string> myIDsList;
        while (std::getline(myIDs, line))
            myIDsList.push_back(line);
        myIDs.close();

        std::ifstream logsFile;
        logsFile.open(LOG_FILENAME, std::fstream::out | std::fstream::in);
        while (std::getline(logsFile, line)) {
            version = {};
            addr_str = "";
            id = {};
            timeStamp = std::to_string(time(NULL));
            // I HATE C++ STRINGS
            std::string accum;
            short counter = 0;
            for (short i = 0; i < line.length() - 1; i++) {
                if (counter == -10)
                    break;
                if (line[i] != ' ')
                    accum += line[i];
                else {
                    switch (counter) {
                        case 0:
                            bdStdLoadNodeId(&id.id, accum);
                            if (accum.length() != 40) {
                                accum = "";
                                counter = -10;
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
                            if (accum[0] == 'B' &&
                                accum[1] == 'D' &&
                                accum[2] == '0' &&
                                accum[3] == '2' &&
                                accum[4] == 'R' &&
                                accum[5] == 'S' &&
                                accum[6] == '5' &&
                                accum[7] == '1') {
                                    memcpy(version.data, accum.c_str(), sizeof(version.data));
                                    counter++;
                            } else
                                counter = -10;
                            accum = "";
                            break;
                        case 3:
                            // Won't work after some time (kinda current time dependent)
                            if (accum.length() >= 10)
                                timeStamp = accum;
                            else
                                timeStamp = std::to_string(time(NULL));
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
                RSPeers[id] = timeStamp;
        }
        logsFile.close();

        //result = new std::list<bdId>[RSPeers.size()]();
        FILE *filtered = fopen(RS_PEERS_FILENAME, "a+");
        for (auto & RSPeer : RSPeers) {
            /*if (result != nullptr)
                result->push_back(it->first);*/
            bdStdPrintNodeId(line, &RSPeer.first.id, false);
            if (std::find(myIDsList.begin(), myIDsList.end(), line) == myIDsList.end()) {
                fprintf(filtered, "%s %s %s\n", line.c_str(),
                        bdnet_inet_ntoa(RSPeer.first.addr.sin_addr).c_str(), RSPeer.second.c_str());
            }
        }
        fclose(filtered);
    }
}

std::list<bdNodeId> Logger::getDiscoveredPeers() {
    std::list<bdNodeId> res;
    for (auto & discoveredPeer : Logger::discoveredPeers)
        res.push_back(discoveredPeer.first);
    return res;
}

