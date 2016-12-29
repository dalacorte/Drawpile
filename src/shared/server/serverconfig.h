/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2016 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QObject>
#include <QString>
#include <QHash>

namespace  server {

class ConfigKey {
public:
	enum Type {
		STRING, // A string value
		TIME,   // Time in seconds, converted from a time definition string
		SIZE,   // (File) size in bytes, converted from a size definition string
		INT,    // Integer
		BOOL    // Boolean (true|1)
	};

	ConfigKey() : index(0), name(nullptr), defaultValue(nullptr), type(STRING) { }
	constexpr ConfigKey(int idx, const char *name, const char *defaultValue, Type type)
		: index(idx), name(name), defaultValue(defaultValue), type(type) { }

	const int index;
	const char *name;
	const char *defaultValue;
	const Type type;
};

namespace config {
	static const ConfigKey
		ClientTimeout(0, "client-timeout", "60", ConfigKey::TIME),            // Connection ping timeout for clients
		SessionSizeLimit(1, "session-size-limit", "15mb", ConfigKey::SIZE),   // Session history size limit in bytes (int)
		SessionCountLimit(2, "session-countlimit", "25", ConfigKey::INT),     // Maximum number of active sessions (int)
		EnablePersistence(3, "persistence", "false", ConfigKey::BOOL),        // Enable session persistence (bool)
		HostPassword(4, "host-password", "", ConfigKey::STRING),              // Session hosting password (string)
		IdleTimeLimit(5, "idle-time-limit", "0", ConfigKey::TIME),            // Session idle time limit in seconds (int)
		ServerTitle(6, "server-title", "", ConfigKey::STRING),                // Server title (string)
		WelcomeMessage(7, "welcome-message", "", ConfigKey::STRING),          // Message sent to a user when they join a session (string)
		AnnounceWhiteList(8, "announce-whitelist", "false", ConfigKey::BOOL), // Should the announcement server whitelist be used (bool)
		LocalAddress(9, "local-address", "", ConfigKey::STRING),              // Local address to use for session listing (string)
		PrivateUserList(10, "private-user-list", "false", ConfigKey::BOOL),   // Don't include user list in announcement (bool)
		RecordingPath(11, "recording-path", "", ConfigKey::STRING),           // Where to put session recordings (string)
		EnableRecording(12, "enable-recording", "false", ConfigKey::BOOL),    // Should sessions be recorded? (bool)
		AllowGuests(13, "allow-guests", "true", ConfigKey::BOOL)              // Allow unauthenticated users (bool)
		;
}

/**
 * @brief Server configuration class
 *
 * These are the configuration settings that can be changed at runtime.
 * The default storage implementation is a simple in-memory key/value map.
 * Deriving classes can implement persistent storage of settings.
 */
class ServerConfig : public QObject
{
	Q_OBJECT
public:
	explicit ServerConfig(QObject *parent=nullptr) : QObject(parent) {}

	// Get configuration values
	QString getConfigString(ConfigKey key) const;
	int getConfigTime(ConfigKey key) const;
	int getConfigSize(ConfigKey key) const;
	int getConfigInt(ConfigKey key) const;
	bool getConfigBool(ConfigKey key) const;
	QVariant getConfigVariant(ConfigKey key) const;

	/**
	 * @brief Set a configuration value
	 * The default implementation sets the value to the in-memory nonpersistent store.
	 *
	 * @param key
	 * @param value
	 */
	virtual void setConfigString(ConfigKey key, const QString &value);

	void setConfigInt(ConfigKey, int value);
	void setConfigBool(ConfigKey, bool value);

	/**
	 * @brief Check if the given listing site URL is allowed
	 *
	 * The default implementation always returns true
	 */
	virtual bool isAllowedAnnouncementUrl(const QUrl &url);

	/**
	 * @brief Parse a time interval string (e.g. "1d" or "5h")
	 * @param str
	 * @return time in seconds or a negative value in case of error
	 */
	static int parseTimeString(const QString &str);

	/**
	 * @brief Parse a file size string (e.g. "10.3 MB" or "550Kb")
	 * @param str
	 * @return size in bytes or a negative value in case of error
	 */
	static int parseSizeString(const QString &str);

protected:
	/**
	 * @brief Get the configuration value for the given key
	 *
	 * The default implementation gets the value from the in-memory nonpersistent
	 * store.
	 * @param key
	 * @param found
	 * @return
	 */
	virtual QString getConfigValue(const ConfigKey key, bool &found) const;

private:
	QHash<int, QString> m_nonpersistent;
};

}

#endif // SERVERCONFIG_H
