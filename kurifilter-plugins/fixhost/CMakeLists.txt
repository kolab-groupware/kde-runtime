set(fixhosturifilter_PART_SRCS fixhosturifilter.cpp )

kde4_add_plugin(fixhosturifilter ${fixhosturifilter_PART_SRCS})

target_link_libraries(fixhosturifilter ${KDE4_KIO_LIBS})

install(TARGETS fixhosturifilter DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES fixhosturifilter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

