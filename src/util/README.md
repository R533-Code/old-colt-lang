# util:
Contains utilities used throughout the front-end of the `colt` compiler.
- `colt_config.h`: Contains CMake configured output, helpful macros for current compiler, platform, version.
- `colt_macro.h`: Contains macro helpers, as `ON_EXIT`, and more.
- `colt_pch.h`: Precompiled header to speedup compilations.
- `dyn_cast.h`: Contains `as`, `dyn_cast`, `is_a` helpers and information about the custom form of `RTTI` used in the front-end.