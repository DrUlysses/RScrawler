#include <zconf.h>
#include "bdiface.h"
#include "bdstddht.h"
#include "bootstrap_fn.h"

void bdSingleSourceFindPeers(BitDhtHandler &dht, short shotsCount, short searchRounds, int regionStart, int regionEnd, std::list<bdNodeId>& toCheckList) {
    std::map<bdNodeId, bdQueryStatus> query;

    bdNodeId searchId;

    /* Search for initial random peers */
    for (int i = 0; i < shotsCount; ++i) {
        /* install search node with random ID */
        bdStdRandomIdFromRegion(&searchId, regionStart, regionEnd);
        bdSingleShotFindPeer(dht, searchId, query);
    }

    for (short i = 0; i < searchRounds; i++) {
        dht.mUdpBitDht->getDhtQueries(query);
        updateQueries(query, dht, toCheckList);
        std::cout << "NUMBER OF IDs TO CHECK: " << toCheckList.size() << std::endl;
        if (toCheckList.size() == 0) {
            std::cout << "Retrying to find any initial peers\n";
            bdStdRandomIdFromRegion(&searchId, regionStart, regionEnd);
            bdSingleShotFindPeer(dht, searchId, query);
            updateQueries(query, dht, toCheckList);
            std::cout << "NUMBER OF IDs TO CHECK: " << toCheckList.size() << std::endl;
        }

    }
}

void bdCheckPeersFromList(BitDhtHandler &dht, std::list<bdNodeId> &toCheckList) {
    std::map<bdNodeId, bdQueryStatus> query;
    std::list<bdNodeId>::iterator toCheckListIterator;
    for (toCheckListIterator = toCheckList.begin(); toCheckListIterator != toCheckList.end(); toCheckListIterator++) {
        if (!dht.getActive() || !dht.getEnabled())
            return;
        bdSingleShotFindPeer(dht, *toCheckListIterator, query);
        updateQueries(query, dht, toCheckList);
        std::cout << "NUMBER OF IDs TO CHECK: " << toCheckList.size() << std::endl;
    }
}

std::map<bdNodeId, std::pair<std::string, time_t>> bdFindPeers(BitDhtHandler &dht, std::list<bdNodeId> peers) {
    std::map<bdNodeId, std::pair<std::string, time_t>> result;
    std::list<bdNodeId>::iterator peersIt;
    for (peersIt = peers.begin(); peersIt != peers.end(); peersIt++)
        result[*peersIt] = bdSingleShotFindStatus(dht, *peersIt);
    return result;
}

std::pair<std::string, time_t> bdSingleShotFindStatus(BitDhtHandler &dht, bdNodeId searchID) {
    std::pair<std::string, time_t> result;
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

    if ((status == BITDHT_QUERY_PEER_UNREACHABLE) ||
        (status == BITDHT_QUERY_SUCCESS)) {
        result.first = dhtStatusToString(status);
        result.second = time(NULL);
    } else {
        std::cerr << "Sorry, Cant be found! ID: ";
        bdStdPrintNodeId(std::cerr, &searchID);
        std::cerr << std::endl;
    }
    return result;
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

        bdQueryStatus qStatus;
        qStatus.mStatus = status;
        qStatus.mResults.push_back(resultId);
        query[searchID] = qStatus;
    } else {
        std::cerr << "Sorry, Cant be found!";
        std::cerr << std::endl;
    }
}

void updateQueries(std::map<bdNodeId, bdQueryStatus> &query, BitDhtHandler &dht, std::list<bdNodeId> &toCheckList) {
    std::ifstream myIDs(USED_IDS_FILENAME);
    if (!myIDs) {
        std::cerr << "Failed to open my IDs file" << std::endl;
        return;
    }
    std::list<std::string> myIDsList;
    std::string line;
    while (std::getline(myIDs, line))
        myIDsList.push_back(line);
    myIDs.close();

    std::string status;
    auto *tempAddr = new sockaddr_in;
    std::multimap<bdMetric, bdPeer>::iterator itIdsMap;
    std::map<bdNodeId, bdQueryStatus>::iterator it;
    bdQuerySummary querySummary;
    for (it = query.begin(); it != query.end(); it++) {
        dht.mUdpBitDht->getDhtQueryStatus(&(*it).first, querySummary);
        status = dhtStatusToString((*it).second.mStatus);
        dht.mUdpBitDht->getDhtPeerAddress(&(*it).first, *tempAddr);
        for (itIdsMap = querySummary.mPotentialPeers.begin();
             itIdsMap != querySummary.mPotentialPeers.end(); itIdsMap++) {
            bdStdPrintNodeId(line, &itIdsMap->second.mPeerId.id, false);
            if (std::find(myIDsList.begin(), myIDsList.end(), line) == myIDsList.end())
                toCheckList.push_back(itIdsMap->second.mPeerId.id);
        }
        for (itIdsMap = querySummary.mClosest.begin(); itIdsMap != querySummary.mClosest.end(); itIdsMap++) {
            bdStdPrintNodeId(line, &itIdsMap->second.mPeerId.id, false);
            if (std::find(myIDsList.begin(), myIDsList.end(), line) == myIDsList.end())
                toCheckList.push_back(itIdsMap->second.mPeerId.id);
        }
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

		





