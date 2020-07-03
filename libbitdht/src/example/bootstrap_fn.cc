#include <zconf.h>
#include "bdiface.h"
#include "bdstddht.h"
#include "bootstrap_fn.h"

#define TICK_LENGTH 1
#define PEER_RECONNECT_TICKS 2

void bdSingleSourceFindPeer(BitDhtHandler &dht, bdNodeId ownID, uint16_t &peer_port, short shotsCount, short searchRounds, int regionStart, int regionEnd) {
    std::map<bdNodeId, bdQueryStatus> query;

    bdNodeId searchId;

    /* Search for initial random peers */
    for (int i = 0; i < shotsCount; ++i) {
        /* install search node with random ID */
        bdStdRandomIdFromRegion(&searchId, regionStart, regionEnd);
        bdSingleShotFindPeer(dht, searchId, query);
    }

    std::cout << "While crawling from ID: ";
    bdStdPrintNodeId(std::cout, &ownID);
    std::cout << std::endl << " Passive found peers: " << std::endl;

    dht.mUdpBitDht->getDhtQueries(query);

    std::list<bdNodeId> toCheckList;
    for (short i = 0; i < searchRounds; i++) {
        printSummary(query, dht, toCheckList);
        std::cout << "NUMBER OF IDs TO CHECK: " << toCheckList.size() << std::endl;
        /*if (toCheckList.size() == 0)
            searchRounds++;*/
        std::list<bdNodeId>::iterator toCheckListIterator;
        for (toCheckListIterator = toCheckList.begin(); toCheckListIterator != toCheckList.end(); toCheckListIterator++) {
            bdSingleShotFindPeer(dht, (*toCheckListIterator), query);
        }
    }

    printSummary(query, dht, toCheckList);
    std::cout << "NUMBER OF IDs TO CHECK: " << toCheckList.size() << std::endl;

    dht.shutdown();
}

void bdSingleShotFindPeer(BitDhtHandler &dht, bdNodeId searchID, std::map<bdNodeId, bdQueryStatus> &query) {
    std::cerr << "bssdht: searching for Id: ";
    bdStdPrintNodeId(std::cerr, &searchID);
    std::cerr << std::endl;

    dht.FindNode(&searchID);

    /* run your program */
    bdId resultId;
    uint32_t status;

    resultId.id = searchID;

    short ticksDone = 0;
    while (!(dht.SearchResult(&resultId, status) || ticksDone >= PEER_RECONNECT_TICKS)) {
        sleep(TICK_LENGTH);
        ticksDone++;
    }

    std::cerr << "bdSingleShotFindPeer() for ID: ";
    bdStdPrintNodeId(std::cerr, &searchID);
    std::cerr << std::endl;
    std::cerr << "Found Result:" << std::endl;

    std::cerr << "\tId: ";
    bdStdPrintId(std::cerr, &resultId);
    std::cerr << std::endl;

    std::cerr << "\tstatus: " << status;
    std::cerr << std::endl;

    if ((status == BITDHT_QUERY_PEER_UNREACHABLE) ||
        (status == BITDHT_QUERY_SUCCESS)) {

        std::string peer_ip = bdnet_inet_ntoa(resultId.addr.sin_addr);
        uint16_t peer_port = ntohs(resultId.addr.sin_port);

        std::cerr << "Answer: ";
        std::cerr << std::endl;
        std::cerr << "\tPeer IpAddress: " << peer_ip;
        std::cerr << std::endl;
        std::cerr << "\tPeer Port: " << peer_port;
        std::cerr << std::endl;

        bdQueryStatus qStatus;
        qStatus.mStatus = status;
        qStatus.mResults.push_back(resultId);
        query[searchID] = qStatus;
    } else {
        std::cerr << "Sorry, Cant be found!";
        std::cerr << std::endl;
    }
}

void printSummary(std::map<bdNodeId, bdQueryStatus> &query, BitDhtHandler &dht, std::list<bdNodeId> &toCheckList) {
    std::string status;
    auto *tempAddr = new sockaddr_in;
    std::multimap<bdMetric, bdPeer>::iterator itIdsMap;
    std::map<bdNodeId, bdQueryStatus>::iterator it;
    bdQuerySummary querySummary;
    for (it = query.begin(); it != query.end(); it++) {
        dht.mUdpBitDht->getDhtQueryStatus(&(*it).first, querySummary);
        bdStdPrintNodeId(std::cout, &(*it).first);
        status = dhtStatusToString((*it).second.mStatus);
        dht.mUdpBitDht->getDhtPeerAddress(&(*it).first, *tempAddr);
        std::cout << " : " << status << std::endl;
        std::cout << "\tfrom IP: " << *tempAddr << std::endl;
        std::cout << "\tquery time: " << querySummary.mQueryTS << std::endl;
        std::cout << "\tsearch time: " << querySummary.mSearchTime << std::endl;
        std::cout << "\tretry period: " << querySummary.mQueryIdlePeerRetryPeriod << std::endl;
        std::cout << std::endl;
        for (itIdsMap = querySummary.mPotentialPeers.begin(); itIdsMap != querySummary.mPotentialPeers.end(); itIdsMap++)
            toCheckList.push_back(itIdsMap->second.mPeerId.id);
        for (itIdsMap = querySummary.mClosest.begin(); itIdsMap != querySummary.mClosest.end(); itIdsMap++)
            toCheckList.push_back(itIdsMap->second.mPeerId.id);
    }
}

std::string dhtStatusToString(uint32_t code) {
    switch (code) {
    case 1:
        return "BITDHT_QUERY_READY";
    case 2:
        return "BITDHT_QUERY_QUERYING";
    case 3:
        return "BITDHT_QUERY_FAILURE";
    case 4:
        return "BITDHT_QUERY_FOUND_CLOSEST";
    case 5:
        return "BITDHT_QUERY_PEER_UNREACHABLE";
    case 6:
        return "BITDHT_QUERY_SUCCESS";
    default:
        return "Unknown";
    }
}

		





