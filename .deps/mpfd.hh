# 1 "mpfd.thh"
// -*- mode: c++ -*-
#ifndef PEQUOD_MPFD_HH
#define PEQUOD_MPFD_HH
#include <tamer/tamer.hh>
#include <tamer/fd.hh>
#include <sys/uio.h>
#include "msgpack.hh"
#include <vector>
#include <deque>

class msgpack_fd {
  public:
    inline msgpack_fd();
    explicit inline msgpack_fd(tamer::fd fd);
    inline msgpack_fd(tamer::fd rfd, tamer::fd wfd);
    ~msgpack_fd();

    inline void initialize(tamer::fd fd);
    void initialize(tamer::fd rfd, tamer::fd wfd);

    void write(const Json& j);
    template <typename R>
    void read_request(tamer::preevent<R, Json> done);
    inline void call(const Json& j, tamer::event<Json> reply);
    void flush(tamer::event<bool> done);
    void flush(tamer::event<> done);
    template <typename R>
    inline void pace(tamer::preevent<R> done);

    inline Json status() const;

  private:
    tamer::fd wfd_;
    tamer::fd rfd_;

    enum { wrcap = 1 << 17, wrlowat = 1 << 12, wrhiwat = wrcap - 2048 };
    struct wrelem {
        StringAccum sa;
        int pos;
    };
    struct flushelem {
        tamer::event<bool> e;
        size_t wpos;
    };
    std::deque<wrelem> wrelem_;
    size_t wrpos_;
    size_t wrsize_;
    bool wrblocked_;
    std::deque<flushelem> flushelem_;
    tamer::event<> wrwake_;
    tamer::event<> wrkill_;

    enum { rdcap = 1 << 17, rdbatch = 1024 };
    String rdbuf_;
    size_t rdpos_;
    size_t rdlen_;
    int rdquota_;
    msgpack::streaming_parser rdparser_;

    struct replyelem {
        tamer::event<Json> e;
        size_t wpos;
    };
    std::deque<tamer::event<Json> > rdreqwait_;
    std::deque<Json> rdreqq_;
    std::deque<replyelem> rdreplywait_;
    unsigned long rdreply_seq_;
    tamer::event<> rdwake_;
    tamer::event<> rdkill_;

    enum { wrpacelim = 1 << 20, rdpacelim = 1 << 14 };
    enum { wrpacerecover = 1 << 19, rdpacerecover = 1 << 13 };
    tamer::event<> pacer_;

    void check() const;
    bool dispatch(bool exit_on_request);
    inline bool read_until_request(bool exit_on_request);
    bool read_one_message();
    void write_once();
    inline bool need_pace() const;
    inline bool pace_recovered() const;
    inline void check_coroutines();
    class closure__writer_coroutine; void  writer_coroutine(); void  writer_coroutine(closure__writer_coroutine &tamer_closure_);
# 83 "mpfd.thh"

    class closure__reader_coroutine; void  reader_coroutine(); void  reader_coroutine(closure__reader_coroutine &tamer_closure_);
# 84 "mpfd.thh"


    msgpack_fd(const msgpack_fd&) = delete;
    msgpack_fd& operator=(const msgpack_fd&) = delete;
    void construct();
};

inline msgpack_fd::msgpack_fd() {
    construct();
}

inline msgpack_fd::msgpack_fd(tamer::fd rfd, tamer::fd wfd) {
    construct();
    initialize(wfd, rfd);
}

inline msgpack_fd::msgpack_fd(tamer::fd fd) {
    construct();
    initialize(fd);
}

inline void msgpack_fd::initialize(tamer::fd fd) {
    initialize(fd, fd);
}

template <typename R>
void msgpack_fd::read_request(tamer::preevent<R, Json> receiver) {
    if (!rdreqq_.empty()) {
        swap(*receiver.result_pointer(), rdreqq_.front());
        rdreqq_.pop_front();
        receiver.unblock();
    } else if (read_until_request(true)) {
        swap(*receiver.result_pointer(), rdparser_.result());
        receiver.unblock();
    } else
        rdreqwait_.push_back(receiver);
}

inline void msgpack_fd::call(const Json& j, tamer::event<Json> done) {
    assert(j.is_a() && j[1].is_i());
    unsigned long seq = j[1].as_i();
    assert(rdreplywait_.empty() || seq == rdreply_seq_ + rdreplywait_.size());
    write(j);
    if (rdreplywait_.empty())
        rdreply_seq_ = seq;
    if (done || !rdreplywait_.empty())
        rdreplywait_.push_back(replyelem{std::move(done), wrpos_ + wrsize_});
    read_until_request(false);
}

inline bool msgpack_fd::read_until_request(bool exit_on_request) {
    while (rdquota_ && read_one_message())
        if (dispatch(exit_on_request))
            return true;
    return false;
}

inline bool msgpack_fd::need_pace() const {
    return wrsize_ > wrpacelim || rdreplywait_.size() > rdpacelim;
}

inline bool msgpack_fd::pace_recovered() const {
    return wrsize_ <= wrpacerecover && rdreplywait_.size() <= rdpacerecover;
}

template <typename R>
inline void msgpack_fd::pace(tamer::preevent<R> done) {
    if (need_pace())
        pacer_ = tamer::distribute(std::move(pacer_), std::move(done));
    else
        done();
}

inline Json msgpack_fd::status() const {
    //check();
    return Json().set("buffered_write_bytes", wrsize_)
        .set("buffered_read_bytes", rdlen_ - rdpos_)
        .set("waiting_readers", rdreqwait_.size() + rdreplywait_.size());
}

#endif
