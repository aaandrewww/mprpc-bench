// -*- mode: c++ -*-
#include "clp.h"
#include "mpfd.hh"
#include <netdb.h>
#include <time.h>
#include <stdio.h>

static bool quiet = false;
tamed void handle_client(tamer::fd cfd);

tamed void server() {
  tvars {
    tamer::fd sfd = tamer::tcp_listen(1234);
    tamer::fd cfd;
  }
  if (sfd)
    std::cerr << "listening on port " << 1234 << std::endl;
  else
    std::cerr << "listen: " << strerror(-sfd.error()) << std::endl;
  while (sfd) {
    twait { sfd.accept(make_event(cfd)); }
    handle_client(cfd);
  }
}

tamed void handle_client(tamer::fd cfd) {
  tvars {
    msgpack_fd mpfd(cfd);
    Json req, res = Json::make_array();
  }

  while (cfd) {
    twait { mpfd.read_request(make_event(req)); }
    if (!req || !req.is_a() || req.size() < 2) {
      if (req)
	std::cerr << "bad RPC: " << req << std::endl;
      break;
    }
    res[0] = -1;
    res[1] = req[1];
    mpfd.write(res);
  }

  cfd.close();
}


tamed void client(int num_msg, int msg_size) {
  tvars {
    tamer::fd cfd;
    msgpack_fd mpfd;
    struct in_addr hostip;
    int i, j;
    Json* req;
    Json* res;
    const char* hostname = "localhost";
    int port = 1234;
    struct hostent* hp = gethostbyname(hostname);
    clock_t t;
    double diff;
    tamer::rendezvous<int> r;
  }

  // lookup hostname address
  if (hp == NULL || hp->h_length != 4 || hp->h_addrtype != AF_INET) {
    std::cerr << "lookup " << hostname << ": " << hstrerror(h_errno) << std::endl;
    return;
  }
  hostip = *((struct in_addr*) hp->h_addr);


  // connect
  twait { tamer::tcp_connect(hostip, port, make_event(cfd)); }
  if (!cfd) {
    std::cerr << "connect " << (hostname ? hostname : "localhost")
	      << ":" << port << ": " << strerror(-cfd.error()) << std::endl;
    return;
  }
  mpfd.initialize(cfd);

  // pingpong 10 times

  res = new Json[num_msg];
  req = new Json[num_msg];

  t = clock();
  for (i = 0; i != num_msg && cfd; ++i) {
    req[i] = Json::array(1, i);

    for (j = 0; j != msg_size; ++j) {
      /* fill the array with j null bytes */
      req[i][j+2] = 0;
    }

    mpfd.call(req[i], tamer::make_event(r, i, res[i]));
  }

  t = clock() - t;
  printf ("%lf\n",((double)t)/CLOCKS_PER_SEC);

  while (r.has_events()) {
    twait(r, i);
    if (!quiet)
      std::cout << "call " << req[i] << ": " << res[i] << std::endl;
  }

  // close out
  cfd.close();
}


static Clp_Option options[] = {
  { "client", 'c', 0, 0, 0 },
  { "listen", 'l', 0, 0, 0 },
  { "num_msg", 'n', 0, Clp_ValInt, 0 },
  { "msg_size", 'm', 0, Clp_ValInt, 0 },
  { "quiet", 'q', 0, 0, Clp_Negate }
};

int main(int argc, char** argv) {
  tamer::initialize();

  bool is_server = false;
  int num_msg = 10;
  int msg_size = 0;
  Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

  while (Clp_Next(clp) != Clp_Done) {
    if (Clp_IsLong(clp, "listen"))
      is_server = true;
    else if (Clp_IsLong(clp, "client"))
      is_server = false;
    else if (Clp_IsLong(clp, "num_msg"))
      num_msg = clp->val.i;
    else if (Clp_IsLong(clp, "quiet"))
      quiet = !clp->negated;
    else if (Clp_IsLong(clp, "msg_size"))
      msg_size = clp->val.i;
  }
  if (is_server)
    server();
  else
    client(num_msg, msg_size);

  tamer::loop();
  tamer::cleanup();
}
