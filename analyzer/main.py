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
            print("Skipped line: " + str(line) + " reason: Wrong entry")
            continue
        elif len(data) >= 3:
            if data[0] in ids:
                print("Skipped line: " + str(line) + " reason: My ID")
                continue
            elif len(data[2]) >= 10:
                res = database.add_entry(data[0], data[1], version, data[2].replace("\n", ''))
                lines_done += 1
            elif len(data) >= 4:
                if len(data[3]) >= 10:
                    if data[2] == "BD02RS51":
                        res = database.add_entry(data[0], data[1], "rs", data[3].replace("\n", ''))
                    elif data[2] == "other":
                        res = database.add_entry(data[0], data[1], "", data[3].replace("\n", ''))
                    else:
                        res = database.add_entry(data[0], data[1], version, data[3].replace("\n", ''))
                    lines_done += 1
        if res:
            print("Done " + str(lines_done) + " lines")
        else:
            print("Skipped line: " + str(line))
    file.close()


def draw_peers_and_time_plot():
    plot.ylabel("Peers count")
    plot.xlabel("Elapsed time")
    data = database.get_all_count_to_time_results()
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("Peers count - Elapsed time.png")
    # # Close, so the pics won't overlap
    # plot.close("all")
    # Save as text file just in case
    with open("Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_unique_peers_and_time_plot():
    plot.ylabel("Unique peers count")
    plot.xlabel("Elapsed time")
    data = database.get_unique_all_count_to_time_results()
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("Unique Peers count - Elapsed time.png")
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    with open("Unique Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_rs_peers_and_time_plot():
    plot.ylabel("RS Peers count")
    plot.xlabel("Elapsed time")
    data = database.get_rs_count_to_time_results()
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("RS Peers count - Elapsed time.png")
    # # Close, so the pics won't overlap
    # plot.close("all")
    # Save as text file just in case
    with open("RS Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


def draw_unique_rs_peers_and_time_plot():
    plot.ylabel("Unique RS peers count")
    plot.xlabel("Elapsed time")
    data = database.get_unique_rs_count_to_time_results()
    plot.bar(range(len(data)), data.values())
    plot.xticks(range(len(data)), data.keys())
    plot.savefig("Unique RS Peers count - Elapsed time.png")
    # Close, so the pics won't overlap
    plot.close("all")
    # Save as text file just in case
    with open("Unique RS Peers count - Elapsed time.txt", 'w') as f:
        print(data, file=f)


if __name__ == '__main__':
    path_to_logs = "../cmake-build-debug"
    # Load my_ids
    ids = load_ids(path_to_logs + "/my_ids")
    # Read the logs
    print("Extracting dhtlogs")
    get_data(path_to_logs + "/dhtlogs", ids)
    print("Extracting rspeers")
    get_data(path_to_logs + "/rspeers", ids, "rs")
    draw_rs_peers_and_time_plot()
    draw_unique_rs_peers_and_time_plot()
    draw_peers_and_time_plot()
    draw_unique_peers_and_time_plot()
