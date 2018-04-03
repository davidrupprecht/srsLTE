/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2018 Software Radio Systems Limited
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

#ifndef SRSEPC_EPC_PARSE_ARGS_H
#define SRSEPC_EPC_PARSE_ARGS_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include "srsepc/hdr/mme/mme.h"
#include "srsepc/hdr/hss/hss.h"
#include "srsepc/hdr/spgw/spgw.h"

#include "srslte/common/bcd_helpers.h"


namespace bpo = boost::program_options;

namespace srsepc {

typedef struct {
  std::string   s1ap_level;
  int           s1ap_hex_limit;
  std::string   gtpc_level;
  int           gtpc_hex_limit;
  std::string   spgw_level;
  int           spgw_hex_limit;
  std::string   hss_level;
  int           hss_hex_limit;
  std::string   all_level;
  int           all_hex_limit;
  std::string   filename;
} log_args_t;


typedef struct{
  mme_args_t   mme_args;
  hss_args_t   hss_args;
  spgw_args_t  spgw_args;
  log_args_t   log_args;
} all_args_t;

void parse_args(all_args_t *args, int argc, char *argv[]);
srslte::LOG_LEVEL_ENUM level(std::string l);
}

#endif // SRSEPC_EPC_PARSE_ARGS_H