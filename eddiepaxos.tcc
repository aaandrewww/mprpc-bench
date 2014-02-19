// -*- mode: c++ -*-
tamed void send_prepare(std::vector<msgpack_fd*> mpfds,
                       uint64_t f,
                       tamer::event<> done) {
   tvars {
       Json req;
       Json* res;
       tamer::rendezvous<unsigned long long> r;
       unsigned long long j;
   }
   res = new Json[f+1];

   for (uint64_t i = 0; i != f+1; ++i) {
       req = Json::array(1, ++message_counter, n_p);
       mpfds[i]->call(req, tamer::make_event(r, i, res[i]));
   }

   while (r.has_waiting()) {
       twait(r, j);
       recv_prepared(res[j][2].as_i(), res[j][3].as_i(), mpfds, f, done);
   }
}

