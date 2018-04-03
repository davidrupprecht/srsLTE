/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2017 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of srsLTE.
 *
 * srsUE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include "srsenb/hdr/enb.h"
#include "srsenb/hdr/misc/metrics_stdout.h"
#include "srsenb/hdr/misc/enb_parse_args.h"

using namespace std;
using namespace srsenb;

static int  sigcnt = 0;
static bool running    = true;
static bool do_metrics = false;

void sig_int_handler(int signo)
{
  sigcnt++;
  running = false;
  printf("Stopping srsENB... Press Ctrl+C %d more times to force stop\n", 10-sigcnt);
  if (sigcnt >= 10) {
    exit(-1);
  }
}

void *input_loop(void *m)
{
  metrics_stdout *metrics = (metrics_stdout*) m;
  char key;
  while(running) {
    cin >> key;
    if (cin.eof() || cin.bad()) {
      cout << "Closing stdin thread." << endl;
      break;
    } else {
      if('t' == key) {
        do_metrics = !do_metrics;
        if(do_metrics) {
          cout << "Enter t to stop trace." << endl;
        } else {
          cout << "Enter t to restart trace." << endl;
        }
        metrics->toggle_print(do_metrics);
      }
    }
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  signal(SIGINT, sig_int_handler);
  signal(SIGTERM, sig_int_handler);
  all_args_t        args;
  metrics_stdout    metrics;
  enb              *enb = enb::get_instance();

  srslte_debug_handle_crash(argc, argv);

  cout << "---  Software Radio Systems LTE eNodeB  ---" << endl << endl;

  parse_args(&args, argc, argv);
  if(!enb->init(&args)) {
    exit(1);
  }
  metrics.init(enb, args.expert.metrics_period_secs);

  pthread_t input;
  pthread_create(&input, NULL, &input_loop, &metrics);

  bool plot_started         = false; 
  bool signals_pregenerated = false; 
  while(running) {
    if (!plot_started && args.gui.enable) {
      enb->start_plot();
      plot_started = true; 
    }
    sleep(1);
  }
  pthread_cancel(input);
  metrics.stop();
  enb->stop();
  enb->cleanup();
  cout << "---  exiting  ---" << endl;
  exit(0);
}
