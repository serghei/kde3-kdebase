#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/khotkeys/shared
  ${CMAKE_SOURCE_DIR}/khotkeys/kcontrol
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### ui (static) ###############################

kde_add_library( ui STATIC_PIC AUTOMOC
  SOURCES
    windowdef_simple_widget_ui.ui windowdef_list_widget_ui.ui
    command_url_widget_ui.ui menuentry_widget_ui.ui
    triggers_tab_ui.ui general_tab_ui.ui action_group_tab_ui.ui
    dcop_widget_ui.ui main_buttons_widget_ui.ui voice_settings_tab_ui.ui
    info_tab_ui.ui window_trigger_widget_ui.ui action_list_widget_ui.ui
    keyboard_input_widget_ui.ui condition_list_widget_ui.ui
    gesture_triggers_tab_ui.ui gestures_settings_tab_ui.ui
    general_settings_tab_ui.ui voice_input_widget_ui.ui
    actions_listview_widget_ui.ui
  LINK khotkeys_shared-shared
)
