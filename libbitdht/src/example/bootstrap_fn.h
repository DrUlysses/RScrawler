
#include <string>
#include <inttypes.h>
#include "bdhandler.h"


/* NOTE. At the moment only the bootstrapfile is actually used.
 * peerId is ignored (a random peerId is searched for). ip & port are not filled in either.
 *
 * This is mainly to finish testing.
 *
 * Once the best form of the return functions is decided (ipv4 structure, or strings).
 * this can be finished off.
 *
 */

void bdSingleSourceFindPeers(BitDhtHandler &dht, short shotsCount, short searchRounds, int regionStart, int regionEnd, std::list<bdNodeId>& toCheckList);
void bdSingleShotFindPeer(BitDhtHandler &dht, bdNodeId searchID, std::map<bdNodeId, bdQueryStatus> &query);
std::map<bdNodeId, std::pair<std::string, time_t>> bdFindPeers(BitDhtHandler &dht, std::list<bdNodeId> peers);
std::pair<std::string, time_t> bdSingleShotFindStatus(BitDhtHandler &dht, bdNodeId searchID);
void bdCheckPeersFromList(BitDhtHandler &dht, std::list<bdNodeId>& toCheckList);

void updateQueries(std::map<bdNodeId, bdQueryStatus> &query, BitDhtHandler &dht, std::list<bdNodeId> &toCheckList);
std::string dhtStatusToString(uint32_t code);
