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

#ifndef FCITXCONFIGUIWRAPPERAPP_H
#define FCITXCONFIGUIWRAPPERAPP_H

#include <QApplication>
#include "mainwindow.h"

class FcitxQtConnection;
class FcitxQtConfigUIFactory;
class WrapperApp : public QApplication {
    Q_OBJECT
public:
    WrapperApp(int& argc, char** argv);
    virtual ~WrapperApp();

private slots:
    void errorExit();
private:
    FcitxQtConfigUIFactory* m_factory;
    MainWindow* m_mainWindow;
};

#endif // FCITXCONFIGUIWRAPPERAPP_H
