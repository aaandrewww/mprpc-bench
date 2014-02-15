# 1 "mpfd.tcc"
// -*- mode: c++ -*-
#include "mpfd.hh"
#include <limits.h>
#include <tamer/adapter.hh>
#ifndef IOV_MAX
#define IOV_MAX 1024
#endif

void msgpack_fd::construct() {
    wrpos_ = 0;
    wrsize_ = 0;
    wrblocked_ = false;
    rdbuf_ = String::make_uninitialized(rdcap);
    rdpos_ = 0;
    rdlen_ = 0;
    rdquota_ = rdbatch;
    rdreply_seq_ = 0;

    wrelem_.push_back(wrelem());
    wrelem_.back().sa.reserve(wrcap);
    wrelem_.back().pos = 0;
}

void msgpack_fd::initialize(tamer::fd wfd, tamer::fd rfd) {
    assert(!wfd_ && !rfd_ && !wrkill_ && !rdkill_ && !wrwake_ && !rdwake_);
    wfd_ = wfd;
    rfd_ = rfd;
    writer_coroutine();
    reader_coroutine();
}

msgpack_fd::~msgpack_fd() {
    wrkill_();
    rdkill_();
    wrwake_();
    rdwake_();
}

void msgpack_fd::write(const Json& j) {
    wrelem* w = &wrelem_.back();
    if (w->sa.length() >= wrhiwat) {
        wrelem_.push_back(wrelem());
        w = &wrelem_.back();
        w->sa.reserve(wrcap);
        w->pos = 0;
    }
    int old_len = w->sa.length();
    msgpack::unparse(w->sa, j);
    wrsize_ += w->sa.length() - old_len;
    wrwake_();
    if (!wrblocked_ && wrelem_.front().sa.length() >= wrlowat)
        write_once();
}

void msgpack_fd::flush(tamer::event<bool> done) {
    if (wrsize_ == 0)
        done(true);
    else
        flushelem_.push_back(flushelem{std::move(done), wrpos_ + wrsize_});
}

void msgpack_fd::flush(tamer::event<> done) {
    flush(tamer::unbind<bool>(done));
}

inline void msgpack_fd::check_coroutines() {
    if (rdquota_ == 0 || !rfd_)
        rdwake_();
    if (!wfd_)
        wrwake_();
}

bool msgpack_fd::read_one_message() {
    assert(rdquota_ != 0);

 readmore:
    // if buffer empty, read more data
    if (rdpos_ == rdlen_) {
        // make new buffer or reuse existing buffer
        if (rdcap - rdpos_ < 4096) {
            if (rdbuf_.is_shared())
                rdbuf_ = String::make_uninitialized(rdcap);
            rdpos_ = rdlen_ = 0;
        }

        ssize_t amt = ::read(rfd_.value(),
                             const_cast<char*>(rdbuf_.data()) + rdpos_,
                             rdcap - rdpos_);

        if (amt != 0 && amt != (ssize_t) -1)
            rdlen_ += amt;
        else {
            if (amt == 0)
                rfd_.close();
            else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                rfd_.close(-errno);
            rdquota_ = 0;
            check_coroutines(); // wake up coroutine [if it's sleeping]
            return false;
        }
    }

    // process new data
    rdpos_ += rdparser_.consume(rdbuf_.begin() + rdpos_, rdlen_ - rdpos_,
                                rdbuf_);

    if (rdparser_.done()) {
        --rdquota_;
        if (rdquota_ == 0)
            rdwake_();          // wake up coroutine [if it's sleeping]
        return true;
    } else
        goto readmore;
}

class msgpack_fd::closure__reader_coroutine : public tamer::tamerpriv::tamer_closure { public:   closure__reader_coroutine (msgpack_fd *__tamer_self) : tamer_closure(tamer_activator_), __tamer_self (__tamer_self), tamer_gather_rendezvous_(this)  {}   static void tamer_activator_(tamer::tamerpriv::tamer_closure *super) {     msgpack_fd::closure__reader_coroutine *self = static_cast<msgpack_fd::closure__reader_coroutine *>(super);     self->__tamer_self->reader_coroutine(*self);   }   msgpack_fd *__tamer_self;     tamer::event<> kill;     tamer::rendezvous<> rendez;   tamer::gather_rendezvous tamer_gather_rendezvous_; }; void  msgpack_fd::reader_coroutine()
{
  closure__reader_coroutine *tamer_closure_ = new closure__reader_coroutine(this);
  tamer_closure_->tamer_activator_(tamer_closure_);
}
void  msgpack_fd::reader_coroutine(closure__reader_coroutine &tamer_closure_)
{
# 116 "mpfd.tcc"

      tamer::event<> &kill TAMER_CLOSUREVARATTR = tamer_closure_.kill;   tamer::rendezvous<> &rendez TAMER_CLOSUREVARATTR = tamer_closure_.rendez;   tamer::tamerpriv::closure_owner<msgpack_fd::closure__reader_coroutine> tamer_closure_holder_(tamer_closure_);   switch (tamer_closure_.tamer_block_position_) {   case 0: break;   case 2:     goto __closure_label_2;     break;   case 3:     goto __closure_label_3;     break;   case 4:     goto __closure_label_4;     break;   default: return; }
# 120 "mpfd.tcc"


    kill = rdkill_ = tamer::make_event(rendez);

    while (kill && rfd_) {
        if (rdquota_ == 0 && rdpos_ != rdlen_)
            /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 126 "mpfd.tcc"
 tamer::at_asap(make_event()); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_2:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 2, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 126 "mpfd.tcc"

#undef make_event
# 126 "mpfd.tcc"

        else if (rdquota_ == 0)
            /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 128 "mpfd.tcc"
 tamer::at_fd_read(rfd_.value(), make_event()); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_3:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 3, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 128 "mpfd.tcc"

#undef make_event
# 128 "mpfd.tcc"

        else if (rdreqwait_.empty() && rdreplywait_.empty())
            /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 130 "mpfd.tcc"
 rdwake_ = make_event(); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_4:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 4, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 130 "mpfd.tcc"

#undef make_event
# 130 "mpfd.tcc"


        rdquota_ = rdbatch;
        while (rdquota_ && (!rdreqwait_.empty() || !rdreplywait_.empty())
               && read_one_message())
            dispatch(false);
        if (pace_recovered())
            pacer_();
    }

    for (auto& e : rdreqwait_)
        e.unblock();
    rdreqwait_.clear();
    for (auto& re : rdreplywait_)
        re.e.unblock();
    rdreplywait_.clear();
    kill();                     // avoid leak of active event
  return;
# 147 "mpfd.tcc"
}

bool msgpack_fd::dispatch(bool exit_on_request) {
    Json& result = rdparser_.result();
    if (!rdparser_.success())
        result = Json();        // XXX reset connection
    rdparser_.reset();
    if (result.is_a() && result[0].is_i() && result[1].is_i()
        && result[0].as_i() < 0) {
        unsigned long seq = result[1].as_i();
        if (seq >= rdreply_seq_ && seq < rdreply_seq_ + rdreplywait_.size()) {
            replyelem& done = rdreplywait_[seq - rdreply_seq_];
            if (done.e.result_pointer())
                swap(*done.e.result_pointer(), result);
            done.e.unblock();
            while (!rdreplywait_.empty() && !rdreplywait_.front().e) {
                rdreplywait_.pop_front();
                ++rdreply_seq_;
            }
        }
        return false;
    } else if (!rdreqwait_.empty()) {
        tamer::event<Json>& done = rdreqwait_.front();
        if (done.result_pointer())
            swap(*done.result_pointer(), result);
        done.unblock();
        rdreqwait_.pop_front();
        return false;
    } else if (exit_on_request)
        return true;
    else {
        rdreqq_.push_back(std::move(result));
        return false;
    }
}

void msgpack_fd::check() const {
    // document invariants
    assert(!wrelem_.empty());
    for (auto& w : wrelem_)
        assert(w.pos <= w.sa.length());
    for (size_t i = 1; i < wrelem_.size(); ++i)
        assert(wrelem_[i].pos == 0);
    for (size_t i = 0; i + 1 < wrelem_.size(); ++i)
        assert(wrelem_[i].pos < wrelem_[i].sa.length());
    if (wrelem_.size() == 1)
        assert(wrelem_[0].pos < wrelem_[0].sa.length()
               || wrelem_[0].sa.empty());
    size_t wrsize = 0;
    for (auto& w : wrelem_)
        wrsize += w.sa.length() - w.pos;
    assert(wrsize == wrsize_);
}

void msgpack_fd::write_once() {
    // check();
    assert(!wrelem_.front().sa.empty());

    struct iovec iov[3];
    int iov_count = (wrelem_.size() > 3 ? 3 : (int) wrelem_.size());
    size_t total = 0;
    for (int i = 0; i != iov_count; ++i) {
        iov[i].iov_base = wrelem_[i].sa.data() + wrelem_[i].pos;
        iov[i].iov_len = wrelem_[i].sa.length() - wrelem_[i].pos;
        total += iov[i].iov_len;
    }

    ssize_t amt = writev(wfd_.value(), iov, iov_count);
    wrblocked_ = amt == 0 || amt == (ssize_t) -1;

    if (amt != 0 && amt != (ssize_t) -1) {
        wrpos_ += amt;
        wrsize_ -= amt;
        while (wrelem_.size() > 1
               && amt >= wrelem_.front().sa.length() - wrelem_.front().pos) {
            amt -= wrelem_.front().sa.length() - wrelem_.front().pos;
            wrelem_.pop_front();
        }
        wrelem_.front().pos += amt;
        if (wrelem_.front().pos == wrelem_.front().sa.length()) {
            assert(wrelem_.size() == 1);
            wrelem_.front().sa.clear();
            wrelem_.front().pos = 0;
        }
        while (!flushelem_.empty()
               && (ssize_t) (wrpos_ - flushelem_.front().wpos) >= 0) {
            flushelem_.front().e.trigger(true);
            flushelem_.pop_front();
        }
        if (pace_recovered())
            pacer_();
    } else {
        if (amt == 0)
            wfd_.close();
        else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            wfd_.close(-errno);
        check_coroutines();     // ensure coroutine is awake
    }
}

class msgpack_fd::closure__writer_coroutine : public tamer::tamerpriv::tamer_closure { public:   closure__writer_coroutine (msgpack_fd *__tamer_self) : tamer_closure(tamer_activator_), __tamer_self (__tamer_self), tamer_gather_rendezvous_(this)  {}   static void tamer_activator_(tamer::tamerpriv::tamer_closure *super) {     msgpack_fd::closure__writer_coroutine *self = static_cast<msgpack_fd::closure__writer_coroutine *>(super);     self->__tamer_self->writer_coroutine(*self);   }   msgpack_fd *__tamer_self;     tamer::event<> kill;     tamer::rendezvous<> rendez;   tamer::gather_rendezvous tamer_gather_rendezvous_; }; void  msgpack_fd::writer_coroutine()
{
  closure__writer_coroutine *tamer_closure_ = new closure__writer_coroutine(this);
  tamer_closure_->tamer_activator_(tamer_closure_);
}
void  msgpack_fd::writer_coroutine(closure__writer_coroutine &tamer_closure_)
{
# 247 "mpfd.tcc"

      tamer::event<> &kill TAMER_CLOSUREVARATTR = tamer_closure_.kill;   tamer::rendezvous<> &rendez TAMER_CLOSUREVARATTR = tamer_closure_.rendez;   tamer::tamerpriv::closure_owner<msgpack_fd::closure__writer_coroutine> tamer_closure_holder_(tamer_closure_);   switch (tamer_closure_.tamer_block_position_) {   case 0: break;   case 2:     goto __closure_label_2;     break;   case 3:     goto __closure_label_3;     break;   default: return; }
# 251 "mpfd.tcc"


    kill = wrkill_ = tamer::make_event(rendez);

    while (kill && wfd_) {
        if (wrelem_.size() == 1 && wrelem_.front().sa.empty())
            /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 257 "mpfd.tcc"
 wrwake_ = make_event(); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_2:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 2, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 257 "mpfd.tcc"

#undef make_event
# 257 "mpfd.tcc"

        else if (wrblocked_) {
            /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 259 "mpfd.tcc"
 tamer::at_fd_write(wfd_.value(), make_event()); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_3:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 3, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 259 "mpfd.tcc"

#undef make_event
# 259 "mpfd.tcc"

            wrblocked_ = false;
        } else
            write_once();
    }

    for (auto& e : flushelem_)
        e.e.trigger((ssize_t) (wrpos_ - e.wpos) >= 0);
    flushelem_.clear();
    for (auto& e : rdreplywait_)
        if ((ssize_t) (wrpos_ - e.wpos) < 0)
            e.e.trigger(Json());
    kill();
  return;
# 272 "mpfd.tcc"
}
