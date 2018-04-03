/**
 *
 * \section COPYRIGHT
 *
 * \section LICENSE
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */
#include <iostream>
#include <fstream>
#include <errno.h>
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include "srslte/common/bcd_helpers.h"
#include "srsepc/hdr/mme/mme.h"
#include "srsepc/hdr/hss/hss.h"
#include "srsepc/hdr/spgw/spgw.h"
#include "srsepc/hdr/misc/epc_parse_args.h"

using namespace std;
using namespace srsepc;

bool running = true;

void 
sig_int_handler(int signo){
  running = false;
}


int
main (int argc,char * argv[] )
{  
  cout << endl <<"---  Software Radio Systems EPC  ---" << endl << endl;
  signal(SIGINT, sig_int_handler);
  signal(SIGTERM, sig_int_handler);
  signal(SIGKILL, sig_int_handler);

  all_args_t args;
  parse_args(&args, argc, argv); 
 
  srslte::logger_stdout logger_stdout;
  srslte::logger_file   logger_file;
  srslte::logger        *logger;

 
  /*Init logger*/
  if (!args.log_args.filename.compare("stdout")) {
    logger = &logger_stdout;
  } else {
    logger_file.init(args.log_args.filename);
    logger_file.log("\n---  Software Radio Systems EPC log ---\n\n");
    logger = &logger_file;
  }

  srslte::log_filter s1ap_log;
  s1ap_log.init("S1AP",logger);
  s1ap_log.set_level(level(args.log_args.s1ap_level));
  s1ap_log.set_hex_limit(args.log_args.s1ap_hex_limit);

  srslte::log_filter mme_gtpc_log;
  mme_gtpc_log.init("GTPC",logger);
  mme_gtpc_log.set_level(level(args.log_args.gtpc_level));
  mme_gtpc_log.set_hex_limit(args.log_args.gtpc_hex_limit);

  srslte::log_filter hss_log;
  hss_log.init("HSS ",logger);
  hss_log.set_level(level(args.log_args.hss_level));
  hss_log.set_hex_limit(args.log_args.hss_hex_limit);

  srslte::log_filter spgw_log;
  spgw_log.init("SPGW",logger);
  spgw_log.set_level(level(args.log_args.spgw_level));
  spgw_log.set_hex_limit(args.log_args.spgw_hex_limit);


  hss *hss = hss::get_instance();
  if (hss->init(&args.hss_args,&hss_log)) {
    cout << "Error initializing HSS" << endl;
    exit(1);
  }

  mme *mme = mme::get_instance();
  if (mme->init(&args.mme_args, &s1ap_log, &mme_gtpc_log, hss)) {
    cout << "Error initializing MME" << endl;
    exit(1);
  }

  spgw *spgw = spgw::get_instance();
  if (spgw->init(&args.spgw_args,&spgw_log)) {
    cout << "Error initializing SP-GW" << endl;
    exit(1);
  } 

  mme->start(); 
  spgw->start();
  while(running) {
    sleep(1);
  }

  mme->stop();
  mme->cleanup();   
  spgw->stop();
  spgw->cleanup();
  hss->stop();
  hss->cleanup();

  cout << std::endl <<"---  exiting  ---" << endl;  
  return 0;
}
