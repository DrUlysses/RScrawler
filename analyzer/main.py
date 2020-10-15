# Logs reader and analysis helper for RS crawler
import database
from os import remove
from matplotlib import pyplot as plot


def load_ids(path):
    # Format is "id(40 chars) ip(any) time(started with 15961) rest(unused here)"
    file = open(path, "r")
    if file.mode != 'r':
        print("Failed to read the file: " + path)
        return
    while True:
        res = []
        line = file.readline()
        if line == '':
            break
        res.append(line)
    return res


class Peer:
    id = ''
    ip = ''
    version = ''
    time = -1


def get_all_data(path, ids, version=""):
    # Format is "id(40 chars) ip(any) time(started with 15961) rest(unused here)"
    file = open(path, "r", newline='', encoding='ISO-8859-1')
    if file.mode != 'r':
        print("Failed to read the file: " + path)
        return
    lines_done = 0
    peers = []
    while True:
        peer = Peer()
        res = False
        line = file.readline().encode('ISO-8859-1').decode('utf-8', 'ignore')
        if line == '':
            break
        data = line.split(' ')
        if len(data) < 3:
            print("Skipped line: " + str(line).replace("\n", '') + " reason: Wrong entry")
            continue
        elif len(data) >= 3:
            if data[0] in ids:
                print("Skipped line: " + str(line).replace("\n", '') + " reason: My ID")
                continue
            elif len(data[2]) >= 10:
                peer.id = data[0]
                peer.ip = data[1]
                peer.version = version
                peer.time = data[2].replace("\n", '')
                peers.append(peer)
                lines_done += 1
            elif len(data) >= 4:
                if len(data[3]) >= 10:
                    if data[2] == "BD02RS51":
                        peer.id = data[0]
                        peer.ip = data[1]
                        peer.version = "rs"
                        peer.time = data[3].replace("\n", '')
                        peers.append(peer)
                        res = True
                    elif data[2] == "other":
                        # res = database.add_entry(data[0], data[1], "", data[3].replace("\n", ''))
                        # print("Skipped line (non-rs-peer) " + str(line))
                        res = False
                    else:
                        peer.id = data[0]
                        peer.ip = data[1]
                        peer.version = version
                        peer.time = data[3].replace("\n", '')
                        peers.append(peer)
                        res = True
                    lines_done += 1
        if res:
            print("Done " + str(lines_done) + " lines")
        else:
            print("Skipped line: " + str(line).replace("\n", ''))
    actual_done = database.add_entries(peers)
    print("Committed " + str(actual_done) + " out of " + str(lines_done))
    file.close()


def get_data(path, ids, version=""):
    # Format is "id(40 chars) ip(any) time(started with 15961) rest(unused here)"
    file = open(path, "r", newline='', encoding='ISO-8859-1')
    if file.mode != 'r':
        print("Failed to read the file: " + path)
        return
    lines_done = 0
    while True:
        res = False
        line = file.readline().encode('ISO-8859-1').decode('utf-8', 'ignore')
        if line == '':
            break
        data = line.split(' ')
        if len(data) < 3:
            print("Skipped line: " + str(line).replace("\n", '') + " reason: Wrong entry")
            continue
        elif len(data) >= 3:
            if data[0] in ids:
                print("Skipped line: " + str(line).replace("\n", '') + " reason: My ID")
                continue
            elif len(data[2]) >= 10:
                res = database.add_entry(data[0], data[1], version, data[2].replace("\n", ''))
                lines_done += 1
            elif len(data) >= 4:
                if len(data[3]) >= 10:
                    if data[2] == "BD02RS51":
                        res = database.add_entry(data[0], data[1], "rs", data[3].replace("\n", ''))
                    elif data[2] == "other":
                        # res = database.add_entry(data[0], data[1], "", data[3].replace("\n", ''))
                        # print("Skipped line (non-rs-peer) " + str(line))
                        res = False
                    else:
                        res = database.add_entry(data[0], data[1], version, data[3].replace("\n", ''))
                    lines_done += 1
        if res:
            print("Done " + str(lines_done) + " lines")
        else:
            print("Skipped line: " + str(line).replace("\n", ''))
    file.close()


def sort_data(data):
    sortednames = sorted(data.keys())
    new_data = dict([])
    for name in sortednames:
        new_data[name] = data[name]
    return new_data


def reverse_keys(data):
    new_values = []
    for value in reversed(data.values()):
        new_values.append(value)
    i = 0
    for key in data.keys():
        data[key] = new_values[i]
        i += 1
    return data


def gen_rs_peers_list(path):
    data = database.get_rs_peers_list()
    open(path, "w").close()
    file = open(path, "w")
    for peer_info in data:
        file.write(peer_info + "\n")
    file.close()


def draw_peers_and_time_plot():
    plot.ylabel("Peers count")
    plot.xlabel("Elapsed time in hours")
    data = reverse_keys(sort_data(database.get_all_count_to_time_results()))
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("Peers count - Elapsed time.pdf")
    # # Close, so the pics won't overlap
    # plot.close("all")
    # Save as text file just in case
    with open("Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_unique_peers_and_time_plot():
    plot.ylabel("Unique peers count")
    plot.xlabel("Elapsed time in hours")
    data = reverse_keys(sort_data(database.get_unique_all_count_to_time_results()))
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("Unique Peers count - Elapsed time.pdf")
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    with open("Unique Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_rs_peers_and_time_plot():
    plot.ylabel("RS Peers count", fontsize=15)
    plot.xlabel("Elapsed time in hours", fontsize=15)
    data = reverse_keys(sort_data(database.get_rs_count_to_time_results()))
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys(), rotation=45, fontsize=15)
    plot.yticks(fontsize=15)
    plot.savefig("RS Peers count - Elapsed time.pdf")
    plot.show()
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    with open("RS Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_unique_rs_peers_and_time_plot():
    plot.ylabel("Unique RS peers count", fontsize=15)
    plot.xlabel("Elapsed time in hours", fontsize=15)
    data = reverse_keys(sort_data(database.get_unique_rs_count_to_time_results()))
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys(), fontsize=15, rotation=45)
    plot.yticks(fontsize=15)
    plot.savefig("Unique RS Peers count - Elapsed time.pdf")
    plot.show()
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    with open("Unique RS Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_unique_rs_peers_and_region_plot():
    plot.ylabel("Unique RS peers count", fontsize=15)
    plot.xlabel("Region", fontsize=15)
    data = reverse_keys(sort_data(database.get_unique_rs_count_to_region_results()))
    space = [i*5 for i in range(len(data))]
    plot.bar(space, data.values(), align="edge", width=5)
    keys = ['00']
    current = 0
    for key in data.keys():
        if current == 15:
            if key == 'f0':
                break
            keys.append(key)
            current = 0
        else:
            current += 1
    # keys.append('ff')
    plot.xticks([i * 80 for i in range(len(keys))], keys, rotation=45, fontsize=15)
    plot.yticks(fontsize=15)
    plot.savefig("Unique RS Peers count - Region.pdf")
    plot.show()
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    # with open("Unique RS Peers count - Region.txt", 'w') as f:
    #     print(new_data, file=f)


if __name__ == '__main__':
    path_to_logs = "../cmake-build-debug"
    # Load my_ids
    ids = load_ids(path_to_logs + "/my_ids")
    # Read the logs
    print("Extracting dhtlogs")
    get_all_data(path_to_logs + "/dhtlogs", ids)
    # print("Extracting rspeers")
    # get_all_data(path_to_logs + "/rspeers", ids, "rs")
    gen_rs_peers_list("new_rs_peers")
    # draw_peers_and_time_plot()
    # draw_unique_peers_and_time_plot()
    # draw_unique_rs_peers_and_time_plot()
    # draw_rs_peers_and_time_plot()
    # draw_unique_rs_peers_and_region_plot()
