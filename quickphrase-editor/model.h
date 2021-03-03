/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef FCITX_TOOLS_GUI_MODEL_H_
#define FCITX_TOOLS_GUI_MODEL_H_

#include <QAbstractTableModel>
#include <QFutureWatcher>
#include <QSet>
#include <QTextStream>

class QFile;
namespace fcitx {

typedef QList<QPair<QString, QString>> QStringPairList;

class QuickPhraseModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit QuickPhraseModel(QObject *parent = 0);
    virtual ~QuickPhraseModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
                         int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const;
    void load(const QString &file, bool append);
    void loadData(QTextStream &stream);
    void addItem(const QString &macro, const QString &word);
    void deleteItem(int row);
    void deleteAllItem();
    QFutureWatcher<bool> *save(const QString &file);
    void saveData(QTextStream &dev);
    bool needSave();

Q_SIGNALS:
    void needSaveChanged(bool m_needSave);

private Q_SLOTS:
    void loadFinished();
    void saveFinished();

private:
    QStringPairList parse(const QString &file);
    bool saveData(const QString &file, const fcitx::QStringPairList &list);
    void setNeedSave(bool needSave);
    bool m_needSave;
    QStringPairList m_list;
    QFutureWatcher<QStringPairList> *m_futureWatcher;
};
} // namespace fcitx

#endif // FCITX_TOOLS_GUI_MODEL_H_
