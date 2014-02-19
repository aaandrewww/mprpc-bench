// -*- mode: c++ -*-
#include "clp.h"
#include "mpfd.hh"
#include <netdb.h> 
#include <stdio.h>
#include <sys/time.h>

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

tamed void client(int num_msg) {
  tvars {
    tamer::fd cfd;
    msgpack_fd mpfd;
    struct in_addr hostip;
    int i;
    Json req, res;
    const char* hostname = "localhost";
    struct hostent* hp = gethostbyname(hostname);
    double diff;
    int port = 1234;
    struct timeval tv[2];
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

  gettimeofday(&tv[0], NULL); 
  twait {
    for (i = 0; i != num_msg && cfd; ++i) {
      req = Json::array(1, i);
      mpfd.call(req, make_event(res));
      if (!quiet)
	std::cout << "call " << req << ": " << res << std::endl;
    }
  }
  gettimeofday(&tv[1], NULL); 

  double elapsed = (tv[1].tv_sec-tv[0].tv_sec) + (tv[1].tv_usec-tv[0].tv_usec)/1000000.0;
  printf ("%lf\n",elapsed);
  // close out
  cfd.close();
}


static Clp_Option options[] = {
  { "client", 'c', 0, 0, 0 },
  { "listen", 'l', 0, 0, 0 },
  { "num_msg", 'n', 0, Clp_ValInt, 0 },
  { "quiet", 'q', 0, 0, Clp_Negate },
  { "port", 'p', 0, Clp_ValInt, 0 },
  { "clients", 't', 0, Clp_ValInt, 0 },
};

int main(int argc, char** argv) {
  tamer::initialize();

  bool is_server = false;
  int num_msg = 10;
  int port = 1234;
  int num_client = 2;
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
    else if (Clp_IsLong(clp, "port"))
      port = clp->val.i;
    else if (Clp_IsLong(clp, "clients"))
      num_client = clp->val.i;
  }
  if (is_server)
    server();
  else
    client(num_msg);

  tamer::loop();
  tamer::cleanup();
}
