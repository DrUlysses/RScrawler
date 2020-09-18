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
    time = Column(Integer)


Base.metadata.create_all(engine)


# Add a song
def add_entry(new_id="", new_ip='', new_time=-1):
    peer = PeerEntry(id=new_id, ip=new_ip, time=new_time)
    old = session.query(PeerEntry).filter_by(time=new_time).first()
    if old is None:
        session.add(peer)
    else:
        old.ip = new_ip
    session.commit()
    print("Added " + new_id)
    return True


def get_count_to_time_results():
    # there should be more efficient way to do this
    # Iterate the database and count the number of results based on the time
    res = dict([])
    entries = session.query(PeerEntry)
    for entry in entries:
        res[entry.time] = res.get(entry.time, 0) + 1
    return res
