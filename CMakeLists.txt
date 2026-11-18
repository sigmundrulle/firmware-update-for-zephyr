# The fw_update module: built into the application when CONFIG_FW_UPDATE is enabled.

if(CONFIG_FW_UPDATE)
  zephyr_include_directories(include)

  zephyr_library()
  zephyr_library_sources(src/fw_update.c)
endif()
