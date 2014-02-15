# 1 "mprpc.tcc"
// -*- mode: c++ -*-
#include "clp.h"
#include "mpfd.hh"
#include <netdb.h>

static bool quiet = false;
class closure__handle_client__Sf; void  handle_client(tamer::fd cfd); void  handle_client(closure__handle_client__Sf &tamer_closure_);
# 7 "mprpc.tcc"


class closure__server__i; void  server(closure__server__i &tamer_closure_);
class closure__server__i : public tamer::tamerpriv::tamer_closure { public:   closure__server__i (TAMER_MOVEARG(int ) port) : tamer_closure(tamer_activator_), port(TAMER_MOVE(port)),
# 11 "mprpc.tcc"
sfd( tamer::tcp_listen(port)), tamer_gather_rendezvous_(this)  {}   static void tamer_activator_(tamer::tamerpriv::tamer_closure *super) {     closure__server__i *self = static_cast<closure__server__i *>(super);     server(*self);   }     int port;     tamer::fd sfd;     tamer::fd cfd;   tamer::gather_rendezvous tamer_gather_rendezvous_; }; 
# 10 "mprpc.tcc"
void  server(int port)
{
  closure__server__i *tamer_closure_ = new closure__server__i(TAMER_MOVE(port));
  tamer_closure_->tamer_activator_(tamer_closure_);
}
void  server(closure__server__i &tamer_closure_)
{
# 9 "mprpc.tcc"

      tamer::fd &sfd TAMER_CLOSUREVARATTR = tamer_closure_.sfd;   tamer::fd &cfd TAMER_CLOSUREVARATTR = tamer_closure_.cfd;   int &port TAMER_CLOSUREVARATTR = tamer_closure_.port;   tamer::tamerpriv::closure_owner<closure__server__i> tamer_closure_holder_(tamer_closure_);   switch (tamer_closure_.tamer_block_position_) {   case 0: break;   case 2:     goto __closure_label_2;     break;   default: return; }
# 13 "mprpc.tcc"

    if (sfd)
        std::cerr << "listening on port " << port << std::endl;
    else
        std::cerr << "listen: " << strerror(-sfd.error()) << std::endl;
    while (sfd) {
        /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 19 "mprpc.tcc"
 sfd.accept(make_event(cfd)); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_2:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 2, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 19 "mprpc.tcc"

#undef make_event
# 19 "mprpc.tcc"

        handle_client(cfd);
    }
  return;
# 22 "mprpc.tcc"
}

class closure__handle_client__Sf; void  handle_client(closure__handle_client__Sf &tamer_closure_);
class closure__handle_client__Sf : public tamer::tamerpriv::tamer_closure { public:   closure__handle_client__Sf (TAMER_MOVEARG(tamer::fd ) cfd) : tamer_closure(tamer_activator_), cfd(TAMER_MOVE(cfd)),
# 26 "mprpc.tcc"
mpfd(cfd),
# 27 "mprpc.tcc"
res( Json::make_array()), tamer_gather_rendezvous_(this)  {}   static void tamer_activator_(tamer::tamerpriv::tamer_closure *super) {     closure__handle_client__Sf *self = static_cast<closure__handle_client__Sf *>(super);     handle_client(*self);   }     tamer::fd cfd;     msgpack_fd mpfd;     Json req;     Json res;   tamer::gather_rendezvous tamer_gather_rendezvous_; }; 
# 25 "mprpc.tcc"
void  handle_client(tamer::fd cfd)
{
  closure__handle_client__Sf *tamer_closure_ = new closure__handle_client__Sf(TAMER_MOVE(cfd));
  tamer_closure_->tamer_activator_(tamer_closure_);
}
void  handle_client(closure__handle_client__Sf &tamer_closure_)
{
# 24 "mprpc.tcc"

      msgpack_fd &mpfd TAMER_CLOSUREVARATTR = tamer_closure_.mpfd;   Json &req TAMER_CLOSUREVARATTR = tamer_closure_.req;   Json &res TAMER_CLOSUREVARATTR = tamer_closure_.res;   tamer::fd &cfd TAMER_CLOSUREVARATTR = tamer_closure_.cfd;   tamer::tamerpriv::closure_owner<closure__handle_client__Sf> tamer_closure_holder_(tamer_closure_);   switch (tamer_closure_.tamer_block_position_) {   case 0: break;   case 2:     goto __closure_label_2;     break;   default: return; }
# 28 "mprpc.tcc"


    while (cfd) {
        /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 31 "mprpc.tcc"
 mpfd.read_request(make_event(req)); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_2:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 2, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 31 "mprpc.tcc"

#undef make_event
# 31 "mprpc.tcc"

        if (!req || !req.is_a() || req.size() < 2 || !req[0].is_i()) {
            if (req)
                std::cerr << "bad RPC: " << req << std::endl;
            break;
        }

        res[0] = -req[0].as_i();
        res[1] = req[1];
        mpfd.write(res);
    }

    cfd.close();
  return;
# 44 "mprpc.tcc"
}


class closure__client__PKci; void  client(closure__client__PKci &tamer_closure_);
class closure__client__PKci : public tamer::tamerpriv::tamer_closure { public:   closure__client__PKci (const char *hostname, TAMER_MOVEARG(int ) port) : tamer_closure(tamer_activator_), hostname(hostname), port(TAMER_MOVE(port)), tamer_gather_rendezvous_(this)  {}   static void tamer_activator_(tamer::tamerpriv::tamer_closure *super) {     closure__client__PKci *self = static_cast<closure__client__PKci *>(super);     client(*self);   }     const char *hostname;     int port;     tamer::fd cfd;     msgpack_fd mpfd;     struct in_addr hostip;     int i;     Json req;     Json res;   tamer::gather_rendezvous tamer_gather_rendezvous_; }; void  client(const char *hostname, int port)
{
  closure__client__PKci *tamer_closure_ = new closure__client__PKci(hostname, TAMER_MOVE(port));
  tamer_closure_->tamer_activator_(tamer_closure_);
}
void  client(closure__client__PKci &tamer_closure_)
{
# 47 "mprpc.tcc"

      tamer::fd &cfd TAMER_CLOSUREVARATTR = tamer_closure_.cfd;   msgpack_fd &mpfd TAMER_CLOSUREVARATTR = tamer_closure_.mpfd;   struct in_addr &hostip TAMER_CLOSUREVARATTR = tamer_closure_.hostip;   int &i TAMER_CLOSUREVARATTR = tamer_closure_.i;   Json &req TAMER_CLOSUREVARATTR = tamer_closure_.req;   Json &res TAMER_CLOSUREVARATTR = tamer_closure_.res;   const char *&hostname TAMER_CLOSUREVARATTR = tamer_closure_.hostname;   int &port TAMER_CLOSUREVARATTR = tamer_closure_.port;   tamer::tamerpriv::closure_owner<closure__client__PKci> tamer_closure_holder_(tamer_closure_);   switch (tamer_closure_.tamer_block_position_) {   case 0: break;   case 2:     goto __closure_label_2;     break;   case 3:     goto __closure_label_3;     break;   default: return; }
# 54 "mprpc.tcc"


    // lookup hostname address
    {
        in_addr_t a = hostname ? inet_addr(hostname) : htonl(INADDR_LOOPBACK);
        if (a != INADDR_NONE)
            hostip.s_addr = a;
        else {
            struct hostent* hp = gethostbyname(hostname);
            if (hp == NULL || hp->h_length != 4 || hp->h_addrtype != AF_INET) {
                std::cerr << "lookup " << hostname << ": " << hstrerror(h_errno) << std::endl;
                return;
            }
            hostip = *((struct in_addr*) hp->h_addr);
        }
    }

    // connect
    /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 72 "mprpc.tcc"
 tamer::tcp_connect(hostip, port, make_event(cfd)); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_2:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 2, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 72 "mprpc.tcc"

#undef make_event
# 72 "mprpc.tcc"

    if (!cfd) {
        std::cerr << "connect " << (hostname ? hostname : "localhost")
                  << ":" << port << ": " << strerror(-cfd.error()) << std::endl;
        return;
    }
    mpfd.initialize(cfd);

    // pingpong 10 times
    for (i = 0; i != 10 && cfd; ++i) {
        req = Json::array(1, i);
        /*twait{*/ do { do {
#define make_event(...) make_event(tamer_closure_.tamer_gather_rendezvous_, ## __VA_ARGS__)
    tamer::tamerpriv::rendezvous_owner<tamer::gather_rendezvous> tamer_gather_rendezvous__holder(tamer_closure_.tamer_gather_rendezvous_);
# 83 "mprpc.tcc"
 mpfd.call(req, make_event(res)); /*}twait*/ tamer_gather_rendezvous__holder.reset(); } while (0); __closure_label_3:   while (tamer_closure_.tamer_gather_rendezvous_.has_waiting()) {       tamer_closure_.tamer_gather_rendezvous_.block(tamer_closure_, 3, __FILE__, __LINE__);       tamer_closure_holder_.reset();       return; }   } while (0);
# 83 "mprpc.tcc"

#undef make_event
# 83 "mprpc.tcc"

        if (!quiet)
            std::cout << "call " << req << ": " << res << std::endl;
    }

    // close out
    cfd.close();
  return;
# 90 "mprpc.tcc"
}


static Clp_Option options[] = {
    { "client", 'c', 0, 0, 0 },
    { "listen", 'l', 0, 0, 0 },
    { "port", 'p', 0, Clp_ValInt, 0 },
    { "host", 'h', 0, Clp_ValString, 0 },
    { "quiet", 'q', 0, 0, Clp_Negate }
};

int main(int argc, char** argv) {
    tamer::initialize();

    bool is_server = false;
    String hostname = "localhost";
    int port = 18029;
    Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

    while (Clp_Next(clp) != Clp_Done) {
        if (Clp_IsLong(clp, "listen"))
            is_server = true;
        else if (Clp_IsLong(clp, "client"))
            is_server = false;
        else if (Clp_IsLong(clp, "port"))
            port = clp->val.i;
        else if (Clp_IsLong(clp, "host"))
            hostname = clp->vstr;
        else if (Clp_IsLong(clp, "quiet"))
            quiet = !clp->negated;
    }

    if (is_server)
        server(port);
    else
        client(hostname.c_str(), port);

    tamer::loop();
    tamer::cleanup();
}
