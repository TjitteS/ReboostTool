/*
    Original copyright 2018 Benjamin Vedder benjamin@vedder.se and the VESC Tool project ( https://github.com/vedderb/vesc_tool )
    Now forked to:
    Tjitte@tpee.nl

    This file is part of Reboost Tool.

    Reboost Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Reboost Tool. is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QDesktopWidget>
#include <QFontDatabase>

#include "mobile/qmlui.h"

int main(int argc, char *argv[])
{
    // Settings
    QCoreApplication::setOrganizationName("TPEE");
    QCoreApplication::setOrganizationDomain("tpee.nl");
    QCoreApplication::setApplicationName(QString("TPEE Reboost Tooool %1").arg(QString::number(DT_VERSION)));


    // DPI settings
    // TODO: http://www.qcustomplot.com/index.php/support/forum/1344

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef USE_MOBILE
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#else
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);

    QSettings set;
    bool scaleAuto = true;
    double scale = 1.0;

    if (set.contains("app_scale_auto")) {
        scaleAuto = set.value("app_scale_auto").toBool();
    } else {
        set.setValue("app_scale_auto", scaleAuto);
    }

    if (scaleAuto) {
        QApplication tmp(argc, argv);
        //QRect rec = tmp.desktop()->screenGeometry();
        QRect rec = tmp.screens()[0]->geometry();

        int height = rec.height();
        int width = rec.width();
        double ptFont = tmp.font().pointSizeF();
        if (ptFont < 0.0) {
            ptFont = tmp.font().pixelSize();
        }

        if (width > 3000 && height > 1700) {
            scale = 1.5;
        } else {
            if (ptFont > 11.0) {
                scale = ptFont / 11.0;
            }
        }

        set.setValue("app_scale_factor", scale);
    } else if (set.contains("app_scale_factor")) {
        scale = set.value("app_scale_factor").toDouble();
    }

    set.setValue("app_scale_factor", scale);

#ifdef Q_OS_ANDROID
    scale = 1.0;
#endif

    if (scale > 1.01) {
        qputenv("QT_SCALE_FACTOR", QString::number(scale).toLocal8Bit());
    }
#endif

    QApplication a(argc, argv);

    // Fonts
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSans.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSans-Bold.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSans-BoldOblique.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSans-Oblique.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSansMono.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSansMono-Bold.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSansMono-BoldOblique.ttf");
    QFontDatabase::addApplicationFont("://res/fonts/DejaVuSansMono-Oblique.ttf");

    qApp->setFont(QFont("DejaVu Sans", 11));

    // Style
    a.setStyleSheet("");
    a.setStyle(QStyleFactory::create("Fusion"));

#ifdef USE_MOBILE
    QmlUi q;
    q.startQmlUi();
#else
    MainWindow w;
    w.show();
#endif

    return a.exec();
}
