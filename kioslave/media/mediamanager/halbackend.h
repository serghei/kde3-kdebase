/* This file is part of the KDE Project
   Copyright (c) 2004-2005 Jérôme Lodewyck <jerome dot lodewyck at normalesup dot org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
* This is a media:/ backend for the freedesktop Hardware Abstraction Layer
* Usage : create an instance of HALBackend, then call InitHal(). A false
* result from the later function means that something went wrong and that
* the backend shall not be used.
*
* @author Jérôme Lodewyck <jerome dot lodewyck at normalesup dot org>
* @short media:/ backend for the HAL
*/

#ifndef _HALBACKEND_H_
#define _HALBACKEND_H_

#include "backendbase.h"

#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>

#include <config.h>

/* We acknowledge the the dbus API is unstable */
#define DBUS_API_SUBJECT_TO_CHANGE
/* DBus-Qt bindings */
#include <dbus/connection.h>
/* HAL libraries */
#include <libhal.h>
#include <libhal-storage.h>

namespace KIO {
  class Job;
}

class HALBackend : public QObject, public BackendBase
{
Q_OBJECT

public:
	/**
	* Constructor
	*/
	HALBackend(MediaList &list, QObject* parent);

	/**
	* Destructor
	*/
	~HALBackend();

	/**
	* Perform HAL initialization.
	*
	* @return true if succeded. If not, rely on some other backend
	*/
	bool InitHal();

	/**
	* List all devices and append them to the media device list (called only once, at startup).
	*
	* @return true if succeded, false otherwise
	*/
	bool ListDevices();

	QStringList mountoptions(const QString &id);

	bool setMountoptions(const QString &id, const QStringList &options);

	QString mount(const QString &id);
	QString mount(const Medium *medium);
	QString unmount(const QString &id);

private:
	/**
	* Append a device in the media list. This function will check if the device
	* is worth listing.
	*
	*  @param udi                 Universal Device Id
	*  @param allowNotification   Indicates if this event will be notified to the user
	*/
	void AddDevice(const char* udi, bool allowNotification=true);

	/**
	* Remove a device from the device list
	*
	*  @param udi                 Universal Device Id
	*/
	void RemoveDevice(const char* udi);

	/**
	* A device has changed, update it
	*
	*  @param udi                 Universal Device Id
	*/
	void ModifyDevice(const char *udi, const char* key);

	/**
	* HAL informed that a special action has occured
	* (e.g. device unplugged without unmounting)
	*
	*  @param udi                 Universal Device Id
	*/
	void DeviceCondition(const char *udi, const char *condition);

	/**
	* Integrate the DBus connection within qt main loop
	*/
	void MainLoopIntegration(DBusConnection *dbusConnection);

/* Set media properties */
private:
	/**
	* Reset properties for the given medium
	*/
	void ResetProperties(const char* MediumUdi, bool allowNotification=false);

	/**
	* Find the medium that is concerned with device udi
	*/
	const char* findMediumUdiFromUdi(const char* udi);

	void setVolumeProperties(Medium* medium);
	bool setFloppyProperties(Medium* medium);
	void setFloppyMountState( Medium* medium );
	bool setFstabProperties(Medium* medium);
	void setCameraProperties(Medium* medium);
	QString generateName(const QString &devNode);
	static QString isInFstab(const Medium *medium);
	static QString listUsingProcesses(const Medium *medium);

private slots:
	void slotResult(KIO::Job *job);

/* Hal call-backs -- from gvm*/
public:
	/** Invoked when a device is added to the Global Device List.
	*
	*  @param  ctx                 LibHal context
	*  @param  udi                 Universal Device Id
	*/
	static void hal_device_added(LibHalContext *ctx, const char *udi);

	/** Invoked when a device is removed from the Global Device List.
	*
	*  @param  ctx                 LibHal context
	*  @param  udi                 Universal Device Id
	*/
	static void hal_device_removed(LibHalContext *ctx, const char *udi);

	/** Invoked when a property of a device in the Global Device List is
	*  changed, and we have we have subscribed to changes for that device.
	*
	*  @param  ctx                 LibHal context
	*  @param  udi                 Univerisal Device Id
	*  @param  key                 Key of property
	*/
	static void hal_device_property_modified(LibHalContext *ctx, const char *udi, const char *key,
				dbus_bool_t is_removed, dbus_bool_t is_added);

	/** Type for callback when a non-continuos condition occurs on a device
	*
	*  @param  udi                 Univerisal Device Id
	*  @param  condition_name      Name of the condition
	*  @param  message             D-BUS message with variable parameters depending on condition
	*/
	static void hal_device_condition(LibHalContext *ctx, const char *udi,
				const char *condition_name,
				const char* message
				);

/* HAL and DBus structures */
private:
	/**
	* The HAL context connecting the whole application to the HAL
	*/
	LibHalContext*		m_halContext;

	/**
	* libhal-storage HAL policy, e.g. for icon names
	*/
	LibHalStoragePolicy*	m_halStoragePolicy;

	/**
	* The DBus-Qt bindings connection for mainloop integration
	*/
	DBusQt::Connection*	m_dBusQtConnection;

	/**
	* Object for the kded module
	*/
	QObject* m_parent;

	DBusConnection *dbus_connection;

	/**
	* Data structure for fstab mount/unmount jobs
	*/
	struct mount_job_data {
		// [in] Medium, which is being mounted/unmounted by the job
		const Medium* medium;
		// [in,out] Should be set to true when the job completes
		bool completed;
		// [out] KIO::Error if an error occured during operation. Otherwise, 0
		int error;
		// [out] Error message to be displayed to the user
		QString errorMessage;
	};

	QMap<KIO::Job *, struct mount_job_data*> mount_jobs;
};

#endif /* _HALBACKEND_H_ */
