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

namespace {
    void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg){
        QByteArray localMsg = msg.toLocal8Bit();
        QString str;
        QString typeStr;
        bool isBad = false;

        switch (type) {
            case QtDebugMsg: typeStr = "DEBUG"; break;
                //    case QtInfoMsg: typeStr = "INFO"; break;
            case QtWarningMsg: typeStr = "WARNING"; isBad = true; break;
            case QtCriticalMsg: typeStr = "CRITICAL"; isBad = true; break;
            case QtFatalMsg: typeStr = "FATAL"; isBad = true; break;

            default:
                break;
        }

        str.sprintf("%s (%s:%u %s): %s", typeStr.toLocal8Bit().data(),
                    context.file, context.line, context.function, localMsg.constData());

        if (PageDebugPrint::currentMsgHandler) {
            QString strTmp;
            if (isBad) {
                strTmp = "<font color=\"red\">" + str + "</font><br>";
            } else {
                strTmp = str + "<br>";
            }

            QMetaObject::invokeMethod(PageDebugPrint::currentMsgHandler, "printConsole",
                                      Qt::QueuedConnection, Q_ARG(QString, strTmp));
        }

        printf("%s\n", str.toLocal8Bit().data());
        fflush(stdout);
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow){
    ui->setupUi(this);

    this->setWindowTitle(QString("TPEE Reboost Tool V%1").arg(QString::number(DT_VERSION)));

    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    mVersion = QString::number(DT_VERSION);
    mMPPT = new MPPTInterface(this);
    mStatusInfoTime = 0;
    mStatusLabel = new QLabel(this);
    ui->statusBar->addPermanentWidget(mStatusLabel);
    mTimer = new QTimer(this);
    mKeyLeft = false;
    mKeyRight = false;
    mMcConfRead = false;

    connect(mTimer, SIGNAL(timeout()),this, SLOT(timerSlot()));
    connect(mMPPT, SIGNAL(statusMessage(QString,bool)),this, SLOT(showStatusInfo(QString,bool)));
    connect(mMPPT, SIGNAL(messageDialog(QString,QString,bool,bool)),this, SLOT(showMessageDialog(QString,QString,bool,bool)));
    connect(mMPPT, SIGNAL(serialPortNotWritable(QString)),this, SLOT(serialPortNotWritable(QString)));
    connect(mMPPT->commands(), SIGNAL(mpptConfigCheckResult(QStringList)),this, SLOT(mpptConfigCheckResult(QStringList)));
    connect(ui->actionAboutQt, SIGNAL(triggered(bool)),qApp, SLOT(aboutQt()));

    // Remove the menu with the option to hide the toolbar
    ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);

    mMPPT->mpptConfig()->loadParamsXml("://res/parameters_mpptConfig.xml");
    mMPPT->infoConfig()->loadParamsXml("://res/info.xml");
    reloadPages();

    connect(mMPPT->mpptConfig(), SIGNAL(updated()),this, SLOT(bmsconfUpdated()));

    qInstallMessageHandler(myMessageOutput);

    mTimer->start(5);

    // Restore size and position
    if (mSettings.contains("mainwindow/size")) {
        resize(mSettings.value("mainwindow/size").toSize());
    }

    if (mSettings.contains("mainwindow/position")) {
        move(mSettings.value("mainwindow/position").toPoint());
    }

    if (mSettings.contains("mainwindow/maximized")) {
        bool maximized = mSettings.value("mainwindow/maximized").toBool();
        if (maximized) {
            showMaximized();
        }
    }

    mPageDebugPrint->printConsole("Reboost Tool " + mVersion + " started<br>");
}

MainWindow::~MainWindow()
{
    // Save settings
    mSettings.setValue("version", mVersion);
    mSettings.setValue("mainwindow/position", pos());
    mSettings.setValue("mainwindow/maximized", isMaximized());

    if (!isMaximized()) {
        mSettings.setValue("mainwindow/size", size());
    }

    delete ui;
}

void MainWindow::timerSlot()
{
    // Update status label
    if (mStatusInfoTime) {
        mStatusInfoTime--;
        if (!mStatusInfoTime) {
            mStatusLabel->setStyleSheet(qApp->styleSheet());
        }
    } else {
        QString str = mMPPT->getConnectedPortName();
        if (str != mStatusLabel->text()) {
            mStatusLabel->setText(mMPPT->getConnectedPortName());
            static QString statusLast = "";
            if (str != statusLast) {
                mPageDebugPrint->printConsole("Status: " + str + "<br>");
                statusLast = str;
            }
        }
    }

    // CAN fwd
    if (ui->actionCanFwd->isChecked() != mMPPT->commands()->getSendCan()) {
        ui->actionCanFwd->setChecked(mMPPT->commands()->getSendCan());
    }

    // RT data only every 10 iterations
    if (ui->actionRtData->isChecked()) {
        static int values_cnt = 0;
        static int cells_cnt = 0;

        values_cnt++;
        if(values_cnt >= 10) {
            values_cnt = 0;
            mMPPT->commands()->getValues();
        }

        //cells_cnt++;
        //if(cells_cnt >= 20) {
        //    cells_cnt = 0;
        //    mMPPT->commands()->getCells();
        //}
    }

    // Send alive command once every 10 iterations
    if (ui->actionSendAlive->isChecked()) {
        static int alive_cnt = 0;
        alive_cnt++;
        if (alive_cnt >= 10) {
            alive_cnt = 0;
            mMPPT->commands()->sendAlive();
        }
    }

    // Read configuration it isn't read since starting VESC Tool
    if (mMPPT->isPortConnected()) {
        static int conf_cnt = 0;
        conf_cnt++;
        if (conf_cnt >= 20) {
            conf_cnt = 0;
            if (!mMcConfRead) {
                mMPPT->commands()->getBMSconf();
            }
        }
    }

    // Disable all data streaming when uploading firmware
    if (mMPPT->commands()->getFirmwareUploadProgress() > 0.1) {
        ui->actionSendAlive->setChecked(false);
        ui->actionRtData->setChecked(false);
    }

    // Run startup checks
    static bool has_run_start_checks = false;
    if (!has_run_start_checks) {
        has_run_start_checks = true;
        checkUdev();
        Utility::checkVersion(mMPPT);
    }
}

void MainWindow::showStatusInfo(QString info, bool isGood)
{
    if (isGood) {
        mStatusLabel->setStyleSheet("QLabel { background-color : lightgreen; color : black; }");
        mPageDebugPrint->printConsole("Status: " + info + "<br>");
    } else {
        mStatusLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
        mPageDebugPrint->printConsole("<font color=\"red\">Status: " + info + "</font><br>");
    }

    mStatusInfoTime = 80;
    mStatusLabel->setText(info);
}

void MainWindow::showMessageDialog(const QString &title, const QString &msg, bool isGood, bool richText)
{
    (void)richText;

    if (isGood) {
        QMessageBox::information(this, title, msg);
    } else {
        QMessageBox::warning(this, title, msg);
    }
}

void MainWindow::serialPortNotWritable(const QString &port)
{
    (void)port;
    return;
    //TODO only do this for linux
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this,
                                 tr("Connect Serial Port"),
                                 tr("The serial port is not writable. This can usually be fixed by "
                                    "adding your user to the dialout, uucp and/or lock groups. Would "
                                    "you like to do that?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply == QMessageBox::Yes) {
        QString name = qgetenv("USER");
        if (name.isEmpty()) {
            name = qgetenv("USERNAME");
        }

        bool hasDialout = !runCmd("getent", QStringList() << "group" << "dialout").isEmpty();
        bool hasUucp = !runCmd("getent", QStringList() << "group" << "uucp").isEmpty();
        bool hasLock = !runCmd("getent", QStringList() << "group" << "lock").isEmpty();

        QString grps;
        if (hasDialout) {
            grps += "dialout";
        }

        if (hasUucp) {
            if (!grps.isEmpty()) {
                grps += ",";
            }
            grps += "uucp";
        }

        if (hasLock) {
            if (!grps.isEmpty()) {
                grps += ",";
            }
            grps += "lock";
        }

        QProcess process;
        process.setEnvironment(QProcess::systemEnvironment());
        process.start("pkexec", QStringList() << "usermod" << "-aG" << grps << name);
        waitProcess(process);
        if (process.exitCode() == 0) {
            showMessageDialog(tr("Command Result"),
                              tr("Result from command:\n\n"
                                 "%1\n"
                                 "You have to reboot for this "
                                 "change to take effect.").
                              arg(QString(process.readAllStandardOutput())),
                              true, false);
        } else {
            showMessageDialog(tr("Command Result"),
                              tr("Running command failed."),
                              false, false);
        }
        process.close();
    }
}

void MainWindow::bmsconfUpdated()
{
    mMcConfRead = true;
}

void MainWindow::mpptConfigCheckResult(QStringList paramsNotSet)
{
    if (!paramsNotSet.isEmpty()) {
        ParamDialog::showParams(tr("Parameters truncated"),
                                tr("The following parameters were truncated because they were set outside "
                                   "of their allowed limits."),
                                mMPPT->mpptConfig(),
                                paramsNotSet,
                                this);
    }
}

void MainWindow::on_actionReconnect_triggered()
{
    mMPPT->reconnectLastPort();
}

void MainWindow::on_actionDisconnect_triggered()
{
    mMPPT->disconnectPort();
}

void MainWindow::on_actionReboot_triggered()
{
    mMPPT->commands()->reboot();
}

void MainWindow::on_actionReadBMScconf_triggered()
{
    mMPPT->commands()->getBMSconf();
}

void MainWindow::on_actionReadBMScconfDefault_triggered()
{
    mMPPT->commands()->getBMSconfDefault();
}

void MainWindow::on_actionWriteBMScconf_triggered()
{
    mMPPT->commands()->setBMSconf();
}

void MainWindow::on_actionSaveBMSConfXml_triggered()
{
    QString path;
    path = QFileDialog::getSaveFileName(this,
                                        tr("Choose where to save the motor configuration XML file"),
                                        ".",
                                        tr("Xml files (*.xml)"));

    if (path.isNull()) {
        return;
    }

    if (!path.toLower().endsWith(".xml")) {
        path += ".xml";
    }

    bool res = mMPPT->mpptConfig()->saveXml(path, "mpptConfiguration");

    if (res) {
        showStatusInfo("Saved motor configuration", true);
    } else {
        showMessageDialog(tr("Save motor configuration"),
                          tr("Could not save motor configuration:<BR>"
                             "%1").arg(mMPPT->mpptConfig()->xmlStatus()),
                          false, false);
    }
}

void MainWindow::on_actionLoadBMSConfXml_triggered()
{
    QString path;
    path = QFileDialog::getOpenFileName(this,
                                        tr("Choose motor configuration file to load"),
                                        ".",
                                        tr("Xml files (*.xml)"));

    if (path.isNull()) {
        return;
    }

    bool res = mMPPT->mpptConfig()->loadXml(path, "mpptConfiguration");

    if (res) {
        showStatusInfo("Loaded motor configuration", true);
    } else {
        showMessageDialog(tr("Load motor configuration"),
                          tr("Could not load motor configuration:<BR>"
                             "%1").arg(mMPPT->mpptConfig()->xmlStatus()),
                          false, false);
    }
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "DieBieMS Tool", Utility::aboutText());
}

void MainWindow::on_actionLibrariesUsed_triggered()
{
    QMessageBox::about(this, "Libraries Used",
                       tr("<b>Icons<br>"
                          "<a href=\"https://icons8.com/\">https://icons8.com/</a><br><br>"
                          "<b>Plotting<br>"
                          "<a href=\"http://qcustomplot.com/\">http://qcustomplot.com/</a>"));
}

void MainWindow::addPageItem(QString name, QString icon, QString groupIcon, bool bold, bool indented)
{
    QListWidgetItem *item = new QListWidgetItem();
    ui->pageList->addItem(item);
    PageListItem *li = new PageListItem(name, icon, groupIcon, this);
    li->setBold(bold);
    li->setIndented(indented);
    ui->pageList->setItemWidget(item, li);
}

void MainWindow::saveParamFileDialog(QString conf, bool wrapIfdef)
{
    ConfigParams *params = 0;

    if (conf.toLower() == "mcconf") {
        params = mMPPT->mpptConfig();
    } else {
        qWarning() << "Invalid conf" << conf;
        return;
    }

    QString path;
    path = QFileDialog::getSaveFileName(this,tr("Choose where to save the configuration header file"),".",tr("h files (*.h)"));

    if (path.isNull()) {
        return;
    }

    if (path.contains(" ")) {
        showMessageDialog(tr("Save header"),tr("Spaces are not allowed in the filename."),false, false);
        return;
    }

    if (!path.toLower().endsWith(".h")) {
        path += ".h";
    }

    bool res = params->saveCDefines(path, wrapIfdef);

    if (res) {
        showStatusInfo("Saved C header", true);
    } else {
        showMessageDialog(tr("Save header"),
                          tr("Could not save header"),
                          false, false);
    }
}

void MainWindow::showPage(const QString &name)
{
    for (int i = 0;i < ui->pageList->count();i++) {
        PageListItem *p = (PageListItem*)(ui->pageList->itemWidget(ui->pageList->item(i)));
        if (p->name() == name) {
            ui->pageList->setCurrentRow(i);
            break;
        }
    }
}

void MainWindow::reloadPages()
{
    // Remove pages (if any)
    ui->pageList->clear();
    while (ui->pageWidget->count() != 0) {
        QWidget* widget = ui->pageWidget->widget(0);
        ui->pageWidget->removeWidget(widget);
        widget->deleteLater();
    }

    mPageWelcome = new PageWelcome(this);
    mPageWelcome->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageWelcome);
    addPageItem(tr("Welcome"), "://res/icons/Home-96.png", "", true);

    mPageConnection = new PageConnection(this);
    mPageConnection->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageConnection);
    addPageItem(tr("Connection"), "://res/icons/Connected-96.png", "", true);

    mPageConfig = new PageConfig(this);
    mPageConfig->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageConfig);
    addPageItem(tr("Configuration"), "://res/icons/Connected-96.png", "", true);

    //mPageCalibration = new PageCalibration(this);
    //mPageCalibration->setMPPT(mMPPT);
    //ui->pageWidget->addWidget(mPageCalibration);
    //addPageItem(tr("Calibration"), "://res/icons/Speed-96.png", "", false,true);

    //mPageconfigSettings = new PageconfigSettings(this);
    //mPageconfigSettings->SetMPPT(mMPPT);
    //ui->pageWidget->addWidget(mPageconfigSettings);
    //addPageItem(tr("Hardware Settings"),"://res/icons/Horizontal Settings Mixer-96.png","",false,true);

    mPageCanSettings = new PageCanSettings(this);
    mPageCanSettings->SetMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageCanSettings);
    addPageItem(tr("User Settings"),"://res/icons/Horizontal Settings Mixer-96.png","",false,true);

    mPageDataAnalysis = new PageDataAnalysis(this);
    mPageDataAnalysis->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageDataAnalysis);
    addPageItem(tr("Data Analysis"), "://res/icons/Line Chart-96.png", "", true);

    mPageRtData = new PageRtData(this);
    mPageRtData->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageRtData);
    addPageItem(tr("Realtime Data"), "://res/icons/rt_off.png", "", false, true);

    mPageSweep = new PageSweep(this);
    mPageSweep->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageSweep);
    addPageItem(tr("Sweep"), "://res/icons/iv.png", "", false, true);

    mPageScope = new PageScope(this);
    mPageScope->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageScope);
    addPageItem(tr("Scope"), "://res/icons/Outgoing Data-96.png", "", false, true);

    mPageTerminal = new PageTerminal(this);
    mPageTerminal->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageTerminal);
    addPageItem(tr("Terminal"), "://res/icons/Console-96.png", "", true);

    mPageDebugPrint = new PageDebugPrint(this);
    ui->pageWidget->addWidget(mPageDebugPrint);
    addPageItem(tr("Debug Console"), "://res/icons/Bug-96.png", "", true);

    //mPageSettings = new PageSettings(this);
    //ui->pageWidget->addWidget(mPageSettings);
    //addPageItem(tr("Tool Settings"), "://res/icons/Settings-96.png", "", true);

    mPageFirmware = new PageFirmware(this);
    mPageFirmware->setMPPT(mMPPT);
    ui->pageWidget->addWidget(mPageFirmware);
    addPageItem(tr("Firmware"), "://res/icons/Electronics-96.png", "", true);

    // Adjust sizes
    QFontMetrics fm(this->font());
    int width = fm.width("Welcome & Wizards++++++");
    int height = fm.height();

    for(int i = 0; i < ui->pageList->count(); i++) {
        QListWidgetItem *item = ui->pageList->item(i);
        item->setSizeHint(QSize(item->sizeHint().width(), height));
    }

    ui->pageList->setMinimumWidth(width);
    ui->pageList->setMaximumWidth(width);
    ui->pageLabel->setMaximumWidth(width);
    ui->pageLabel->setMaximumHeight((394 * width) / 1549);

    ui->pageList->setCurrentRow(0);
    ui->pageWidget->setCurrentIndex(0);
}

void MainWindow::checkUdev()
{
    return;
#ifdef _OS_Linux_Linux
    // Check if udev rules for modemmanager are installed

    QFileInfo fi_mm("/lib/udev/rules.d/77-mm-usb-device-blacklist.rules");
    if (fi_mm.exists()) {
        QFileInfo fi_bms("/lib/udev/rules.d/45-vesc.rules");
        if (!fi_vesc.exists()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this,
                                             tr("Modemmenager"),
                                             tr("It looks like modemmanager is installed on your system, and that "
                                                "there are no VESC udev rules installed. This will cause a delay "
                                                "from when you plug in the VESC until you can use it. Would you like "
                                                "to add a udev rule to prevent modemmanager from grabbing the VESC?"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (reply == QMessageBox::Yes) {
                QFile f_bms(QDir::temp().absoluteFilePath(fi_vesc.fileName()));
                if (!f_vesc.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    showMessageDialog(tr("Create File Error"),
                                      f_vesc.errorString(),
                                      false, false);
                    return;
                }

                f_vesc.write("# Prevent modemmanager from grabbing the VESC\n"
                             "ATTRS{idVendor}==\"0483\", ATTRS{idProduct}==\"5740\", ENV{ID_MM_DEVICE_IGNORE}=\"1\"\n");
                f_vesc.close();

                QFileInfo fi_new(f_vesc);
                QProcess process;
                process.setEnvironment(QProcess::systemEnvironment());
                process.start("pkexec", QStringList() <<
                              "mv" <<
                              fi_new.absoluteFilePath() <<
                              fi_vesc.absolutePath());
                waitProcess(process);

                if (process.exitCode() == 0) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::information(this,
                                                     tr("Modemmenager"),
                                                     tr("The udev rule was created successfully. Would you like "
                                                        "to reload udev to apply the new rule?"),
                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                    if (reply == QMessageBox::Yes) {
                        QProcess process;
                        process.setEnvironment(QProcess::systemEnvironment());
                        process.start("pkexec", QStringList() <<
                                      "udevadm" <<
                                      "control" <<
                                      "--reload-rules");
                        waitProcess(process);

                        if (process.exitCode() == 0) {
                            showMessageDialog(tr("Command Result"),
                                              tr("Reloaded udev rules sucessfully."),
                                              true, false);
                        } else {
                            QString out = process.readAll();

                            if (out.isEmpty()) {
                                showMessageDialog(tr("Command Result"),
                                                  tr("Could not reload udev rules. A reboot is probably "
                                                     "required for this change to take effect."),
                                                  false, false);
                            }
                        }
                        process.close();
                    }
                } else {
                    showMessageDialog(tr("Command Result"),
                                      tr("Could not move rules file:\n\n"
                                         "%1").
                                      arg(QString(process.readAllStandardOutput())),
                                      false, false);
                }
                process.close();
            }
        }
    }
#endif
}

bool MainWindow::waitProcess(QProcess &process, bool block, int timeoutMs)
{
    bool wasEnables = isEnabled();
    bool killed = false;

    if (block) {
        setEnabled(false);
    }

    process.waitForStarted();

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(timeoutMs);
    connect(&process, SIGNAL(finished(int)), &loop, SLOT(quit()));
    connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();

    if (process.state() == QProcess::Running) {
        process.kill();
        process.waitForFinished();
        killed = true;
    }

    setEnabled(wasEnables);

    return !killed;
}

QString MainWindow::runCmd(QString cmd, QStringList args)
{
    QProcess process;
    process.setEnvironment(QProcess::systemEnvironment());
    process.start(cmd, args);
    waitProcess(process);
    QString res = process.readAllStandardOutput();
    process.close();
    return res;
}

void MainWindow::on_pageList_currentRowChanged(int currentRow)
{
    if (currentRow >= 0) {
        ui->pageWidget->setCurrentIndex(currentRow);
    }
}

void MainWindow::on_actionParameterEditorBMScconf_triggered()
{
    ParameterEditor *p = new ParameterEditor(this);
    p->setAttribute(Qt::WA_DeleteOnClose);
    p->setParams(mMPPT->mpptConfig());
    p->show();
}

void MainWindow::on_actionParameterEditorInfo_triggered()
{
    ParameterEditor *p = new ParameterEditor(this);
    p->setAttribute(Qt::WA_DeleteOnClose);
    p->setParams(mMPPT->infoConfig());
    p->show();
}

void MainWindow::on_actionSavempptConfigurationHeader_triggered()
{
    saveParamFileDialog("mcconf", false);
}

void MainWindow::on_actionSavempptConfigurationHeaderWrap_triggered()
{
    saveParamFileDialog("mcconf", true);
}

void MainWindow::on_actionTerminalPrintFaults_triggered()
{
    mMPPT->commands()->sendTerminalCmd("faults");
    showPage("DieBieMS Terminal");
}

void MainWindow::on_actionTerminalShowHelp_triggered()
{
    mMPPT->commands()->sendTerminalCmd("help");
    showPage("DieBieMS Terminal");
}

void MainWindow::on_actionTerminalClear_triggered()
{
    mPageTerminal->clearTerminal();
    showPage("DieBieMS Terminal");
}

void MainWindow::on_actionTerminalPrintThreads_triggered()
{
    mMPPT->commands()->sendTerminalCmd("threads");
    showPage("DieBieMS Terminal");
}

void MainWindow::on_actionCanFwd_toggled(bool arg1)
{
    mMPPT->commands()->setSendCan(arg1);
}

void MainWindow::on_actionSafetyInformation_triggered()
{
    HelpDialog::showHelp(this, mMPPT->infoConfig(), "wizard_startup_usage");
}

void MainWindow::on_actionWarrantyStatement_triggered()
{
    HelpDialog::showHelp(this, mMPPT->infoConfig(), "wizard_startup_warranty");
}

void MainWindow::on_actionDieBieMSToolChangelog_triggered()
{
    HelpDialog::showHelp(this, "Reboost Tool Changelog", Utility::vescToolChangeLog());
}

void MainWindow::on_actionFirmwareChangelog_triggered()
{
    HelpDialog::showHelp(this, "Firmware Changelog", Utility::fwChangeLog());
}

void MainWindow::on_actionLicense_triggered()
{
    HelpDialog::showHelp(this, mMPPT->infoConfig(), "gpl_text");
}

void MainWindow::on_actionStoreBMScconf_triggered()
{
    mMPPT->commands()->storempptConfig();
}
