MITK_CREATE_MODULE(
  PACKAGE_DEPENDS
    PUBLIC Boost Qt5|Core DCMTK
)

if (UNIX)
  set_target_properties(${MODULE_TARGET} PROPERTIES LINK_FLAGS "-lpthread")
endif (UNIX)
