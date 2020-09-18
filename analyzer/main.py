# Logs reader and analysis helper for RS crawler
import database
from os import remove
from matplotlib import pyplot as plot


def get_data(path):
    # Format is "id(40 chars) ip(any) time(started with 15961) rest(unused here)"
    file = open(path, "r", newline='', encoding='ISO-8859-1')
    if file.mode != 'r':
        print("Failed to read file: " + path)
        return
    while True:
        line = file.readline().encode('ISO-8859-1').decode('utf-8', 'ignore')
        if line == '':
            break
        data = line.split(' ')
        if data[0].count('0') == 40:
            break
        if len(data[0]) == 40 and data[1].count('.') == 3:
            if data[2].startswith("16"):
                database.add_entry(data[0], data[1], data[2].replace("\n", ''))
            elif data[3].startswith("16"):
                database.add_entry(data[0], data[1], data[3].replace("\n", ''))
    file.close()


def draw_plot():
    plot.ylabel("Peers count")
    plot.xlabel("Elapsed time")
    data = database.get_count_to_time_results()
    plot.bar(*zip(*data.items()))
    plot.savefig("Peers count - Elapsed time.png")
    plot.show()


if __name__ == '__main__':
    # Read the logs
    path_to_logs = "/home/ulysses/RS/rs-crawler/cmake-build-debug/"
    print("Extracting dhtlogs")
    get_data(path_to_logs + "dhtlogs")
    print("Extracting rspeers")
    get_data(path_to_logs + "rspeers")
    draw_plot()
