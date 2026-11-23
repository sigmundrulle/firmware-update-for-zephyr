# The fuz module: built into the application when CONFIG_FUZ is enabled.

if(CONFIG_FUZ)
  zephyr_include_directories(include)

  zephyr_library()
  zephyr_library_sources(src/fuz.c)
endif()
