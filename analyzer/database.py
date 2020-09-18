from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base

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


# Add a song
def add_entry(new_id="", new_ip='', new_version="", new_time=-1):
    peer = PeerEntry(id=new_id, ip=new_ip, version=new_version, time=new_time)
    old = session.query(PeerEntry).filter_by(time=new_time).first()
    if old is None:
        session.add(peer)
    else:
        old.ip = new_ip
    session.commit()
    return True


def get_all_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of results based on the time
    res = dict([])
    entries = session.query(PeerEntry)
    for entry in entries:
        time = entry.time
        res[time] = res.get(time, 0) + 1
    return res


def get_rs_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of rs results based on the time
    res = dict([])
    entries = session.query(PeerEntry)
    for entry in entries:
        if entry.version == "rs":
            time = entry.time
            res[time] = res.get(time, 0) + 1
    return res