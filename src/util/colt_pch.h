/** @file colt_pch.h
* Colt precompiled header, contains includes used throughout multiple files.
*/

#ifndef HG_COLT_PCH
#define HG_COLT_PCH

#include <cstdlib>

#include <utility>
#include <fstream>
#include <filesystem>

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
#include <fmt/chrono.h>

//Colt-Structs includes
#define COLT_USE_FMT
#include <colt/data_structs/Expected.h>
#include <colt/data_structs/Optional.h>
#include <colt/data_structs/Vector.h>
#include <colt/data_structs/String.h>
#include <colt/data_structs/Map.h>
#include <colt/data_structs/UniquePtr.h>
#include <colt/data_structs/Set.h>
#include <colt/utility/Typedefs.h>

//Includes
#include <util/colt_config.h>

#include <util/dyn_cast.h>

#include <cmd/colt_args.h>

#include <util/colt_macro.h>
#include <util/colt_print.h>

#endif //!HG_COLT_PCH