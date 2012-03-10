#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4, this, SLOT(fnSlot) )
# define DEF2( name, key3, key4, receiver, slot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4, receiver, slot );
#else
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4 )
# define DEF2( name, key3, key4, receiver, slot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4 )
#endif
#define WIN KKey::QtWIN

	keys->insert( "Program:kdesktop", i18n("Desktop") );

#ifndef NOSLOTS
	if (kapp->authorize("run_command"))
	{
#endif
		DEF( I18N_NOOP("Run Command"), ALT+Qt::Key_F2, WIN+Qt::Key_Return, slotExecuteCommand() );
#ifndef NOSLOTS
	}
#endif
	DEF( I18N_NOOP("Show Taskmanager"), CTRL+Qt::Key_Escape, WIN+CTRL+Qt::Key_Pause, slotShowTaskManager() );
	DEF( I18N_NOOP("Show Window List"), ALT+Qt::Key_F5, WIN+Qt::Key_0, slotShowWindowList() );
	DEF( I18N_NOOP("Switch User"), ALT+CTRL+Qt::Key_Insert, WIN+Qt::Key_Insert, slotSwitchUser() );
#ifndef NOSLOTS
	if (kapp->authorize("lock_screen"))
	{
#endif
		DEF2( I18N_NOOP("Lock Session"), ALT+CTRL+Qt::Key_L, WIN+Qt::Key_ScrollLock, KRootWm::self(), SLOT(slotLock()) );
#ifndef NOSLOTS
	}
	if (kapp->authorize("logout"))
	{
#endif
		DEF( I18N_NOOP("Log Out"), ALT+CTRL+Qt::Key_Delete, WIN+Qt::Key_Escape, slotLogout() );
		DEF( I18N_NOOP("Log Out Without Confirmation"), ALT+CTRL+SHIFT+Qt::Key_Delete, WIN+SHIFT+Qt::Key_Escape, slotLogoutNoCnf() );
		DEF( I18N_NOOP("Halt without Confirmation"), ALT+CTRL+SHIFT+Qt::Key_PageDown, WIN+CTRL+SHIFT+Qt::Key_PageDown, slotHaltNoCnf() );
		DEF( I18N_NOOP("Reboot without Confirmation"), ALT+CTRL+SHIFT+Qt::Key_PageUp, WIN+CTRL+SHIFT+Qt::Key_PageUp, slotRebootNoCnf() );
#ifndef NOSLOTS
	}
#endif

#undef DEF
#undef DEF2
#undef WIN
