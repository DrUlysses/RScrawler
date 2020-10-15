from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from copy import deepcopy

engine = create_engine("sqlite:///" + "base.db", echo=False,
                       connect_args={'check_same_thread': False})
Session = sessionmaker(bind=engine)
session = Session()
Base = declarative_base()


class PeerEntry(Base):
    __tablename__ = "lines"
    number = Column(Integer, primary_key=True)
    id = Column(String)
    ip = Column(String)
    version = Column(String)
    time = Column(Integer)


Base.metadata.create_all(engine)


# Add a peer
def add_entry(new_id="", new_ip='', new_version="", new_time=""):
    if new_id.count('0') == 40 or len(new_id) != 40 or new_ip.count('.') != 3 or not new_time.isnumeric():
        return False
    peer = PeerEntry(id=new_id, ip=new_ip, version=new_version, time=new_time)
    old = session.query(PeerEntry).filter_by(id=new_id).first()
    if old is None:
        session.add(peer)
        session.commit()
        return True
    elif old.time != new_time:
        session.add(peer)
        session.commit()
        return True
    return False


# Add multiple peers
def add_entries(entries):
    res = 0
    overall = len(entries)
    for entry in entries:
        if entry.id.count('0') == 40 or len(entry.id) != 40 or entry.ip.count('.') != 3 \
                or not entry.time.isnumeric() or entry.version != "rs":
            continue
        peer = PeerEntry(id=entry.id, ip=entry.ip, version=entry.version, time=entry.time)
        old = session.query(PeerEntry).filter_by(id=entry.id).first()
        if old is None:
            session.add(peer)
            res += 1
        elif old.time != entry.time:
            session.add(peer)
            res += 1
        print("Added " + str(res) + " entries")
    session.commit()
    return overall - res


def get_rs_peers_list():
    res = []
    entered_peers = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs" and entry.id not in entered_peers:
            res.append(str(entry.id) + " " + str(entry.ip) + " BD02RS51 " + str(entry.time))
            entered_peers.append(entry.id)
    return res


def sort_dict_by_time(dictionary, are_numbers=True):
    biggest = int(max(dictionary.keys()))
    # Merge near values
    res = dict([])
    removed = []
    for key, value in dictionary.items():
        key = int(key)
        if are_numbers:
            value = int(value)
        new_key = key
        new_value = value
        for temp_key, temp_value in dictionary.items():
            if temp_key != key:
                if int(abs(temp_key - key)) < 1800 and temp_key not in removed:
                    new_value += temp_value
                    if temp_key < new_key:
                        if new_key not in removed:
                            removed.append(new_key)
                        new_key = temp_key
                    else:
                        removed.append(temp_key)
        if new_key not in removed:
            removed.append(new_key)
            res[new_key] = new_value
    # Simplify values
    dictionary = deepcopy(res)
    for key in dictionary:
        delta = biggest - int(key)
        if delta > biggest:
            delta = 0
        res[int(delta / 3600)] = res.pop(key)
    return res


def get_all_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of results based on the time
    res = dict([])
    found = {}
    entries = session.query(PeerEntry)
    for entry in entries:
        time = int(str(entry.time)[0:10])
        if time not in found.keys():
            found[time] = []
        if entry.id not in found[time]:
            found[time].append(entry.id)
    temp_res = sort_dict_by_time(found, False)
    for key in temp_res.keys():
        entities = []
        for value in temp_res[key]:
            if value not in entities:
                entities.append(value)
                current = res.get(key, 0)
                res[key] = current + 1
    return res


def get_unique_all_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    ids = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.id not in ids:
            time = int(str(entry.time)[0:10])
            current = res.get(time, 0)
            res[time] = current + 1
            ids.append(entry.id)
    return sort_dict_by_time(res)


def get_rs_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    found = {}
    entries = session.query(PeerEntry)
    for entry in entries:
        temp = int(entry.time)
        time = str(temp)[0:10]
        time = int(time)
        if entry.version == "rs":
            if time not in found.keys():
                found[time] = []
            if entry.id not in found[time]:
                found[time].append(entry.id)
    temp_res = sort_dict_by_time(found, False)
    for key in temp_res.keys():
        entities = []
        for value in temp_res[key]:
            if value not in entities:
                entities.append(value)
                current = res.get(key, 0)
                res[key] = current + 1
    return res


def get_unique_rs_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    ids = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs" and entry.id not in ids:
            time = int(str(entry.time)[0:10])
            current = res.get(time, 0)
            res[time] = current + 1
            ids.append(entry.id)
    return sort_dict_by_time(res)


def get_unique_rs_count_to_region_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    letters = "0123456789abcdef"
    for i in range(0, 15):
        for j in range(0, 15):
            res[letters[i] + letters[j]] = 0
    ids = []
    # ips = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs" and entry.id not in ids:
            region = entry.id[0] + entry.id[1]
            # if entry.ip not in ips:
            #     ips.append(entry.ip)
            current = res.get(region, 0)
            res[region] = current + 1
            ids.append(entry.id)
    return res
