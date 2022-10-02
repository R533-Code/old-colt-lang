/** @file colt_pch.h
* Colt precompiled header, contains includes used throughout multiple files.
*/

#ifndef HG_COLT_PCH
#define HG_COLT_PCH

#include <cstdlib>

#include <utility>
#include <fstream>

#include <limits>

#include <chrono>

#include <mutex>
#include <atomic>
#include <thread>

#include <algorithm>
#include <charconv>

//{fmt} includes
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>

//Colt-Structs includes
#include <colt/Expected.h>
#include <colt/Optional.h>
#include <colt/Vector.h>
#include <colt/String.h>
#include <colt/Map.h>

//Includes
#include <util/colt_config.h>

#include <util/colt_macro.h>
#include <util/colt_print.h>
#include <util/typedefs.h>

#endif //!HG_COLT_PCH