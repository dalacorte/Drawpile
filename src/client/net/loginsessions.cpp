/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2014 Calle Laakkonen

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QDebug>
#include <QPixmap>

#include "loginsessions.h"

namespace net {

LoginSessionModel::LoginSessionModel(QObject *parent) :
	QAbstractTableModel(parent)
{
}

int LoginSessionModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	return _sessions.size();
}

int LoginSessionModel::columnCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	return 3;
}

QVariant LoginSessionModel::data(const QModelIndex &index, int role) const
{

	const LoginSession &ls = _sessions.at(index.row());

	if(role == Qt::DisplayRole) {
		switch(index.column()) {
		case 1: return ls.title;
		case 2: return ls.userCount;
		}
	} else if(role == Qt::DecorationRole) {
		switch(index.column()) {
		case 0:
			if(ls.closed)
				return QPixmap(":icons/lock_closed.png");
			break;
		}
	}

	return QVariant();
}

QVariant LoginSessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch(section) {
	case 1: return tr("Title");
	case 2: return tr("Users");
	}

	return QVariant();
}

void LoginSessionModel::updateSession(const LoginSession &session)
{
	int oldIndex=-1;
	for(int i=0;i<_sessions.size();++i) {
		if(_sessions.at(i).id == session.id) {
			oldIndex = i;
			break;
		}
	}

	if(oldIndex>=0) {
		_sessions[oldIndex] = session;
		emit dataChanged(index(oldIndex, 0), index(oldIndex, columnCount()));
	} else {
		beginInsertRows(QModelIndex(), _sessions.size(), _sessions.size());
		_sessions.append(session);
		endInsertRows();
	}
}

void LoginSessionModel::removeSession(int id)
{
	int idx=-1;
	for(int i=0;i<_sessions.size();++i) {
		if(_sessions.at(i).id == id) {
			idx = i;
			break;
		}
	}
	if(idx<0) {
		qWarning() << "removeSession: id" << id << "does not exist!";
	} else {
		beginRemoveRows(QModelIndex(), idx, idx);
		_sessions.removeAt(idx);
		endRemoveRows();
	}
}

}
