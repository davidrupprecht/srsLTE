/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of the srsUE library.
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

#include "srsue/hdr/ue.h"
#include "srsue/hdr/misc/ue_parse_args.h"
#include "srslte/srslte.h"
#include "srsue/hdr/misc/metrics_stdout.h"
#include "srsue/hdr/misc/metrics_csv.h"
#include "srslte/common/metrics_hub.h"
#include "srslte/version.h"

using namespace std;
using namespace srsue;

/**********************************************************************
 *  Program arguments processing
 ***********************************************************************/


static int sigcnt = 0;
static bool running = true;
static bool do_metrics = false;
metrics_stdout metrics_screen;

void sig_int_handler(int signo) {
  sigcnt++;
  running = false;
  printf("Stopping srsUE... Press Ctrl+C %d more times to force stop\n", 10-sigcnt);
  if (sigcnt >= 10) {
    exit(-1);
  }
}

void *input_loop(void *m) {
  char key;
  while (running) {
    cin >> key;
    if (cin.eof() || cin.bad()) {
      cout << "Closing stdin thread." << endl;
      break;
    } else {
      if ('t' == key) {
        do_metrics = !do_metrics;
        if (do_metrics) {
          cout << "Enter t to stop trace." << endl;
        } else {
          cout << "Enter t to restart trace." << endl;
        }
        metrics_screen.toggle_print(do_metrics);
      } else
      if ('q' == key) {
        running = false;
      }
    }
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  srslte::metrics_hub<ue_metrics_t> metricshub;
  signal(SIGINT, sig_int_handler);
  signal(SIGTERM, sig_int_handler);
  all_args_t args;

  srslte_debug_handle_crash(argc, argv);

  parse_args(&args, argc, argv);

  srsue_instance_type_t type = LTE;
  ue_base *ue = ue_base::get_instance(type);
  if (!ue) {
    cout << "Error creating UE instance." << endl << endl;
    exit(1);
  }

  cout << "---  Software Radio Systems " << srsue_instance_type_text[type] << " UE  ---" << endl << endl;
  if (!ue->init(&args)) {
    exit(1);
  }

  metricshub.init(ue, args.expert.metrics_period_secs);
  metricshub.add_listener(&metrics_screen);
  metrics_screen.set_ue_handle(ue);

  metrics_csv metrics_file(args.expert.metrics_csv_filename);
  if (args.expert.metrics_csv_enable) {
    metricshub.add_listener(&metrics_file);
    metrics_file.set_ue_handle(ue);
  }

  pthread_t input;
  pthread_create(&input, NULL, &input_loop, &args);

  bool plot_started = false;
  bool signals_pregenerated = false;

  while (running) {
    if (ue->is_attached()) {
      if (!signals_pregenerated && args.expert.pregenerate_signals) {
        ue->pregenerate_signals(true);
        signals_pregenerated = true;
      }
      if (!plot_started && args.gui.enable) {
        ue->start_plot();
        plot_started = true;
      }
    }
    sleep(1);
  }
  pthread_cancel(input);
  metricshub.stop();
  ue->stop();
  ue->cleanup();
  cout << "---  exiting  ---" << endl;
  exit(0);
}
