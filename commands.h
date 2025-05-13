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
#ifndef COMMANDS_H
#define COMMANDS_H

#include <QObject>
#include <QTimer>
#include "vbytearray.h"
#include "datatypes.h"
#include "packet.h"
#include "configparams.h"

class Commands : public QObject
{
    Q_OBJECT
public:
    explicit Commands(QObject *parent = 0);

    void setLimitedMode(bool is_limited);
    Q_INVOKABLE bool isLimitedMode();
    Q_INVOKABLE void setSendCan(bool sendCan, int id = -1);
    Q_INVOKABLE bool getSendCan();
    Q_INVOKABLE void setCanSendId(unsigned int id);
    Q_INVOKABLE int getCanSendId();
    void setmpptConfig(ConfigParams *mpptConfig);
    Q_INVOKABLE void startFirmwareUpload(QByteArray &newFirmware, bool isBootloader = false);
    double getFirmwareUploadProgress();
    QString getFirmwareUploadStatus();
    Q_INVOKABLE void cancelFirmwareUpload();
    void checkmpptConfig();
    void storempptConfig();

signals:
    void dataToSend(QByteArray &data);
    void fwVersionReceived(int major, int minor, QString hw, QByteArray uuid);
    void ackReceived(QString ackType);
    void valuesReceived(MPPT_VALUES values);
    void sweepDataReceifed(QVector<double> vs, QVector<double> is);

    void cellsReceived(int cellCount, QVector<double> cellVoltageArray);
    void sweepReceifed(QVector<double> vs, QVector<double>is);
    void printReceived(QString str);
    void rotorPosReceived(double pos);
    void mpptConfigCheckResult(QStringList paramsNotSet);

    void scopeDataReady();
    void scopeDataReceived(int channel, int index, QVector<double> data);
    void scopeSampleRateReceived(float samplerate);

public slots:
    void processPacket(QByteArray data);
    void getFwVersion();
    void getValues();
    void getCells();
    void getSweep(double start, double end);
    void sendTerminalCmd(QString cmd);
    void setDetect(disp_pos_mode mode);
    void samplePrint(debug_sampling_mode mode, int sample_len, int dec);
    void getBMSconf();
    void getBMSconfDefault();
    void setBMSconf(bool check = true);
    void reboot();
    void sendAlive();
    void pairNrf(int ms);

    void ScopeRun(int samples, int pretrigger, int div, int ch1source, int channel2source, bool onFault);
    void ScopeCurrentStep(int samples, int pretrigger, int div, int ch1source, int channel2source, float I0, float I1);
    void ScopeGetData();
    void ScopeDataPollReady();


private slots:
    void timerSlot();

private:
    void emitData(QByteArray data);
    void firmwareUploadUpdate(bool isTimeout);
    QString opStateToStr(OperationalStateTypedef fault);
    QString phaseFaultToStr(PhaseFault_t mode);
    QString phaseModeToStr(PhaseMode_t mode);

    QTimer *mTimer;
    bool mSendCan;
    int mCanId;
    bool mIsLimitedMode;

    // FW upload state
    QByteArray mNewFirmware;
    bool mFirmwareIsUploading;
    int mFirmwareState;
    int mFimwarePtr;
    int mFirmwareTimer;
    int mFirmwareRetries;
    bool mFirmwareIsBootloader;
    QString mFirmwareUploadStatus;

    ConfigParams *mmpptConfig;
    ConfigParams mmpptConfigLast;
    bool mCheckNextmpptConfig;

    int mTimeoutCount;
    int mTimeoutFwVer;
    int mTimeoutBMSconf;
    int mTimeoutValues;
    int mTimeoutCells;
    int mTimeoutSweep;
    int mTimeoutScope;
};

#endif // COMMANDS_H
