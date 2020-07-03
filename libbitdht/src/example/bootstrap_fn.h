
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

void bdSingleSourceFindPeer(BitDhtHandler &dht, bdNodeId ownID, uint16_t &port, short shotsCount, short searchRounds, int regionStart, int regionEnd);
void bdSingleShotFindPeer(BitDhtHandler &dht, bdNodeId searchID, std::map<bdNodeId, bdQueryStatus> &query);

void printSummary(std::map<bdNodeId, bdQueryStatus> &query, BitDhtHandler &dht, std::list<bdNodeId> &toCheckList);
std::string dhtStatusToString(uint32_t code);
