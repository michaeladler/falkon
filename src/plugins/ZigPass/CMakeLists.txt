add_definitions(${ZIGPASS_CFLAGS})

set( ZigPass_SRCS
  zigpass_plugin.cpp
  zigpass_backend.cpp
	)

ecm_create_qm_loader( ZigPass_SRCS falkon_zigpass_qt )

set( ZigPass_RSCS
  zigpass.qrc
	)
qt5_add_resources(RSCS ${ZigPass_RSCS})

add_library(ZigPass MODULE ${ZigPass_SRCS} ${RSCS})
target_link_libraries(ZigPass "zigpass")

install(TARGETS ZigPass DESTINATION ${FALKON_INSTALL_PLUGINDIR})
target_link_libraries(ZigPass FalkonPrivate)

