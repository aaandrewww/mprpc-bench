// -*- mode: c++ -*-
#include "clp.h"
#include "mpfd.hh"
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>    // std::max
#include <vector>       // std::vector

static bool quiet = false;
tamed void handle_client(tamer::fd cfd);

/*
 * An RPC message is a JSON Array [procedure, id, args ...]
 *
 * where
 *
 *   - procedure is a signed int corresponding to a remote procedure
 *
 *   - id is an int corresponding to the message id
 *
 *   - args is any number of arguments to the procedure call
 *
 */

/*
 * 1 prepare(n)      -> prepared(n_a, v_a)
 * 2 accept(n_p v_o) -> accepted(n_a)
 * 3 decided(v_o)    -> ok()
 */

/* persistent state */
static uint64_t n_p = 0;
static uint64_t n_l = 0;
static uint64_t n_a = 0;
static uint64_t v_a = 0;
/* volatile state */
static uint64_t uid = rand();
static uint64_t a;
static uint64_t n_o;
static uint64_t v_o;
static uint64_t decisions; /* keep track of number of decided values */
static uint64_t message_counter = 0;

/******************************************************************************
 * Declarations
 */

tamer::fd setup_connection(const char* hostname, uint64_t port);
uint64_t choose_value();

void handle_prepare(uint64_t n, Json* res);
void handle_accept(uint64_t n, uint64_t v, Json* res);
void handle_decided(uint64_t v);

tamed void propose(std::vector<msgpack_fd*> mpfds, uint64_t f, tamer::event<> done);
tamed void send_prepare(std::vector<msgpack_fd*> mpfds, uint64_t f, tamer::event<> done);
tamed void recv_prepared(uint64_t n,
                         uint64_t v,
                         std::vector<msgpack_fd*> mpfds,
                         uint64_t f,
                         tamer::event<> done);
tamed void send_accept(std::vector<msgpack_fd*> mpfds,
                       uint64_t f,
                       tamer::event<> done);
tamed void recv_accepted(uint64_t n,
                         std::vector<msgpack_fd*> mpfds,
                         uint64_t f,
                         tamer::event<> done);
tamed void send_decided(std::vector<msgpack_fd*> mpfds,
                        uint64_t f,
                        tamer::event<> done);

/******************************************************************************
 * Acceptor Code
 */

tamed void acceptor(const char* hostname, uint64_t port) {
  tvars {
    tamer::fd cfd = setup_connection(hostname, port);
    msgpack_fd mpfd;
    Json req, res = Json::make_array();
  }

  if (!cfd) {
    std::cerr << "Failed to open tcp connection to "
              << hostname << ":" << port << std::endl;
    return;
  }

  mpfd.initialize(cfd);

  while (cfd) {
    twait { mpfd.read_request(tamer::make_event(req)); }
    if (!req || !req.is_a() || req.size() < 2) {
      if (req)
        std::cerr << "bad RPC: " << req << std::endl;
      break;
    }
    res[0] = -req[0].as_i();
    res[1] = req[1];

    switch (req[0].as_i()) {
    case 1:
      /* 1 prepare(n)      -> prepared(n_a, v_a) */
      handle_prepare(req[2].as_i(), &res);
      break;
    case 2:
      /* 2 accept(n_p v_o) -> accepted(n_a) */
      handle_accept(req[2].as_i(), req[3].as_i(), &res);
      break;
    case 3:
      /* 3 decided(v_o)    -> ok() */
      handle_decided(req[2].as_i());
      break;
    default:
      std::cerr << "bad RPC procedure id: " << req[0].as_i() << std::endl;
      break;
    }

    mpfd.write(res);
  }

  cfd.close();
}

void handle_prepare(uint64_t n, Json* res) {
  n_l = std::max(n_l, n);
  res[2] = n_a;
  res[3] = v_a;
}

void handle_accept(uint64_t n, uint64_t v, Json* res) {
  if (n >= n_l) {
    n_l = n_a = n;
    v_a = v;
  }
  res[2] = n_a;
}

void handle_decided(uint64_t v) {
  ++decisions;
  n_p = 0;
  n_l = 0;
  n_a = 0;
  v_a = 0;
  std::cout << "paxos: quorom reached on value: " << v << std::endl;
}


/******************************************************************************
 * Proposer code
 */

tamed void proposer(const char* hostname, uint64_t port, uint64_t f) {
  tvars {
    tamer::fd cfd;
    msgpack_fd* mpfd;
    std::vector<tamer::fd> cfds;
    std::vector<msgpack_fd*> mpfds;
  }

  for (uint64_t i = 0; i != 2*f+1; ++i) {
    cfd = setup_connection(hostname, port+i);
    cfds.push_back(cfd);

    if (!cfd) {
      std::cerr << "Failed to open tcp connection to "
                << hostname << ":" << port << std::endl;
      return;
    }

    mpfd = new msgpack_fd();
    mpfd->initialize(cfd);
    mpfds.push_back(mpfd);
  }

  twait {
    propose(mpfds, f, tamer::make_event());
  }

  /* clean up tamer fds */
  for (uint64_t i = 0; i != 2*f+1; ++i) {
    cfds[i].close();
  }
}

tamed void propose(std::vector<msgpack_fd*> mpfds, uint64_t f, tamer::event<> done) {
  n_p = n_p + 1 + uid;
  a = 0;
  n_o = 0;
  send_prepare(mpfds, f, done);
}

tamed void send_prepare(std::vector<msgpack_fd*> mpfds, uint64_t f, tamer::event<> done) {
  tvars {
    Json req;
    Json* res;
    tamer::rendezvous<uint64_t> r;
    uint64_t j;
  }
  res = new Json[f+1];

  for (uint64_t i = 0; i != f+1; ++i) {
    req = Json::array(1, ++message_counter, n_p);
    mpfds[i]->call(req, tamer::make_event<Json>(r, /* the tamer::rendezvous */
                                                i, /* the event id */
                                                res[i] /* where to return the value to */
                                                ));
  }

  while (r.has_waiting()) {
    twait(r, &j);
    recv_prepared(res[j][2].as_i(), res[j][3].as_i(), mpfds, f, done);
  }
}

tamed void recv_prepared(uint64_t n,
                         uint64_t v,
                         std::vector<msgpack_fd*> mpfds,
                         uint64_t f,
                         tamer::event<> done) {
  if (n > n_o) {
    n_o = n;
    v_o = v;
  }
  a = a + 1;
  if (a == f+1) {
    if (v_o == 0) {
      v_o = choose_value();
    }
    a = 0;
    n_p = std::max(n_p, n_o);
    send_accept(mpfds, f, done);
  }
}

tamed void send_accept(std::vector<msgpack_fd*> mpfds,
                       uint64_t f,
                       tamer::event<> done) {
  tvars {
    Json req;
    Json* res;
    tamer::rendezvous<uint64_t> r;
    uint64_t j;
  }
  res = new Json[f+1];

  for (uint64_t i = 0; i != f+1; ++i) {
    req = Json::array(2, ++message_counter, n_p);
    mpfds[i]->call(req, tamer::make_event<Json>(r, /* the tamer::rendezvous */
                                                i, /* the event id */
                                                res[i] /* where to return the value to */
                                                ));
  }

  while (r.has_waiting() != 0) {
    twait(r, &j);
    recv_accepted(res[j][2].as_i(), mpfds, f, done);
  }
}

tamed void recv_accepted(uint64_t n,
                         std::vector<msgpack_fd*> mpfds,
                         uint64_t f,
                         tamer::event<> done) {
  if (n == n_p) {
    a = a + 1;
  }
  if (a == f+1) {
    send_decided(mpfds, f, done);
  }
}

tamed void send_decided(std::vector<msgpack_fd*> mpfds,
                        uint64_t f,
                        tamer::event<> done) {
  tvars {
    Json req;
    Json* res;
    uint64_t j;
  }
  res = new Json[2*f+1];

  /* we wait until everyone confirms the decided before proposing again */
  twait {
    for (uint64_t i = 0; i != 2*f+1; ++i) {
      req = Json::array(3, ++message_counter, n_p);
      mpfds[i]->call(req, tamer::make_event<Json>(res[i]));
    }
  }

  /* goes all the way back to client() */
  done.trigger();
}

uint64_t choose_value() {
  return rand();
}



tamer::fd setup_connection(const char* hostname, uint64_t port) {
  tamer::fd cfd;
  struct in_addr hostip;
  Json req, res;
  struct hostent* hp = gethostbyname(hostname);

  // lookup hostname address
  if (hp == NULL || hp->h_length != 4 || hp->h_addrtype != AF_INET) {
    std::cerr << "lookup " << hostname << ": " << hstrerror(h_errno) << std::endl;
    return;
  }
  hostip = *((struct in_addr*) hp->h_addr);


  // connect
  twait { tamer::tcp_connect(hostip, port, tamer::make_event(cfd)); }
  if (!cfd) {
    std::cerr << "connect " << (hostname ? hostname : "localhost")
	      << ":" << port << ": " << strerror(-cfd.error()) << std::endl;
    return (tamer::fd)NULL;
  }

  return cfd;
}

static Clp_Option options[] = {
  { "proposer", 'r', 0, 0, 0 },
  { "acceptor", 'a', 0, 0, 0 },
  { "port", 'p', 0, Clp_ValInt, 0 },
  { "hostname", 'h', 0, Clp_ValStringNotOption, 0 },
  { "failures", 'f', 0, Clp_ValInt, 0 },
  { "quiet", 'q', 0, 0, Clp_Negate }
};

int main(int argc, char** argv) {
  tamer::initialize();

  bool is_acceptor = false;
  const char* hostname = "localhost";
  uint64_t port = 1234;
  uint64_t f = 0;
  Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

  while (Clp_Next(clp) != Clp_Done) {
    if (Clp_IsLong(clp, "acceptor"))
      is_acceptor = true;
    else if (Clp_IsLong(clp, "proposer"))
      is_acceptor = false;
    else if (Clp_IsLong(clp, "hostname"))
      hostname = clp->val.s;
    else if (Clp_IsLong(clp, "quiet"))
      quiet = !clp->negated;
    else if (Clp_IsLong(clp, "port"))
      port = clp->val.i;
    else if (Clp_IsLong(clp, "failures"))
      f = clp->val.i;
  }

  if(f<=0) {
    std::cerr << "paxos: failures must be at least 1, given " << f << std::endl;
    return -1;
  }

  if (is_acceptor)
    acceptor(hostname, port);
  else
    proposer(hostname, port, f);

  tamer::loop();
  tamer::cleanup();
}
