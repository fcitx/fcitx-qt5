/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *   Copyright (C) 2017~2017 by xzhao                                      *
 *   i@xuzhao.net                                                          *
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

#ifndef FCITXQT5_GUIWRAPPER_MAINWINDOW_H
#define FCITXQT5_GUIWRAPPER_MAINWINDOW_H

#include <QMainWindow>

#include "fcitxqtconfiguiwidget.h"
#include "ui_mainwindow.h"

class FcitxQtInputMethodProxy;
class FcitxQtConnection;
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(FcitxQtConfigUIWidget *pluginWidget,
                        QWidget *parent = 0);
    virtual ~MainWindow();
public Q_SLOTS:
    void changed(bool changed);
    void clicked(QAbstractButton *button);
    void connected();
    void saveFinished();

private:
    Ui::MainWindow *m_ui;
    FcitxQtConnection *m_connection;
    FcitxQtConfigUIWidget *m_pluginWidget;
    FcitxQtInputMethodProxy *m_proxy;
};

#endif // FCITXQT5_GUIWRAPPER_MAINWINDOW_H
