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
        if entry.id.count('0') == 40 or len(entry.id) != 40 or entry.ip.count('.') != 3 or not entry.time.isnumeric():
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


def sort_dict(dictionary):
    biggest = 0
    for key in dictionary:
        if key > biggest:
            biggest = key
    # Merge near values
    res = dict([])
    removed = []
    for key, value in dictionary.items():
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
        delta = biggest - key
        if delta > biggest:
            delta = 0
        res[int(delta / 3600)] = res.pop(key)
    return res


def get_all_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of results based on the time
    res = dict([])
    entries = session.query(PeerEntry)
    for entry in entries:
        time = entry.time
        current = res.get(time, 0)
        res[time] = current + 1
    return sort_dict(res)


def get_unique_all_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    ids = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.id not in ids:
            time = entry.time
            current = res.get(time, 0)
            res[time] = current + 1
            ids.append(entry.id)
    return sort_dict(res)


def get_rs_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs":
            time = entry.time
            current = res.get(time, 0)
            res[time] = current + 1
    return sort_dict(res)


def get_unique_rs_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    ids = []
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs" and entry.id not in ids:
            time = entry.time
            current = res.get(time, 0)
            res[time] = current + 1
            ids.append(entry.id)
    return sort_dict(res)
