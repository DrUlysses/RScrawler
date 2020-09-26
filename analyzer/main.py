# Logs reader and analysis helper for RS crawler
import database
from os import remove
from matplotlib import pyplot as plot


def get_data(path, version=""):
    # Format is "id(40 chars) ip(any) time(started with 15961) rest(unused here)"
    file = open(path, "r", newline='', encoding='ISO-8859-1')
    if file.mode != 'r':
        print("Failed to read the file: " + path)
        return
    lines_done = 0
    while True:
        line = file.readline().encode('ISO-8859-1').decode('utf-8', 'ignore')
        if line == '':
            break
        data = line.split(' ')
        if data[0].count('0') == 40:
            continue
        if len(data[0]) == 40 and data[1].count('.') == 3:
            if len(data) >= 3:
                if len(data[2]) >= 10:
                    database.add_entry(data[0], data[1], version, data[2].replace("\n", ''))
                    lines_done += 1
                elif len(data) >= 4:
                    if len(data[3]) >= 10:
                        database.add_entry(data[0], data[1], version, data[3].replace("\n", ''))
                        lines_done += 1
        print("Done " + str(lines_done) + " lines")
    file.close()


def draw_peers_and_time_plot():
    plot.ylabel("Peers count")
    plot.xlabel("Elapsed time")
    data = database.get_all_count_to_time_results()
    plot.bar(*zip(*data.items()))
    plot.savefig("Peers count - Elapsed time.png")


def draw_rs_peers_and_time_plot():
    plot.ylabel("Peers count")
    plot.xlabel("Elapsed time")
    data = database.get_rs_count_to_time_results()
    plot.bar(*zip(*data.items()))
    plot.savefig("RS Peers count - Elapsed time.png")


if __name__ == '__main__':
    # Read the logs
    path_to_logs = "../cmake-build-debug"
    print("Extracting rspeers")
    get_data(path_to_logs + "/rspeers", "rs")
    draw_rs_peers_and_time_plot()
    print("Extracting dhtlogs")
    get_data(path_to_logs + "/dhtlogs")
    draw_peers_and_time_plot()
