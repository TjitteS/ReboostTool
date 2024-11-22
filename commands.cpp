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

#include "commands.h"
#include <QDebug>
#include <QSettings>

Commands::Commands(QObject *parent) : QObject(parent)
{
    mSendCan = QSettings().value("send_can", false).toBool();
    mCanId = QSettings().value("can_id", 0).toInt();
    mIsLimitedMode = false;

    // Firmware state
    mFirmwareIsUploading = false;
    mFirmwareState = 0;
    mFimwarePtr = 0;
    mFirmwareTimer = 0;
    mFirmwareRetries = 0;
    mFirmwareUploadStatus = "FW Upload Status";
    mCheckNextmpptConfig = false;

    mTimer = new QTimer(this);
    mTimer->setInterval(10);
    mTimer->start();

    mmpptConfig = 0;

    mTimeoutCount = 50;
    mTimeoutFwVer = 0;
    mTimeoutBMSconf = 0;
    mTimeoutValues = 0;
    mTimeoutCells = 0;

    connect(mTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
}

void Commands::setLimitedMode(bool is_limited)
{
    mIsLimitedMode = is_limited;
}

bool Commands::isLimitedMode()
{
    return mIsLimitedMode;
}

void Commands::setSendCan(bool sendCan, int id)
{
    mSendCan = sendCan;
    QSettings().setValue("send_can", sendCan);

    if (id >= 0) {
        setCanSendId(id);
    }
}

bool Commands::getSendCan()
{
    return mSendCan;
}

void Commands::setCanSendId(unsigned int id)
{
    mCanId = id;
    QSettings().setValue("can_id", id);
}

int Commands::getCanSendId()
{
    return mCanId;
}

void Commands::processPacket(QByteArray data)
{
    VByteArray vb(data);
    COMM_PACKET_ID id = (COMM_PACKET_ID)vb.vbPopFrontUint8();

    switch (id) {
    case COMM_FW_VERSION: {
        mTimeoutFwVer = 0;
        int fw_major;
        int fw_minor;
        QString hw;
        QByteArray uuid;

        if (vb.size() >= 2) {
            fw_major = vb.vbPopFrontInt8();
            fw_minor = vb.vbPopFrontInt8();
            hw = vb.vbPopFrontString();
            if (vb.size() >= 12) {
                uuid.append(vb.left(12));
                vb.remove(0, 12);
            }
        } else {
            fw_major = -1;
            fw_minor = -1;
        }

        emit fwVersionReceived(fw_major, fw_minor, hw, uuid);
    } break;

    case COMM_ERASE_NEW_APP:
    case COMM_WRITE_NEW_APP_DATA:
        firmwareUploadUpdate(!vb.at(0));
        break;

    case COMM_GET_VALUES: {
        mTimeoutValues = 0;
        MPPT_VALUES values;

        values.Iin                = vb.vbPopFrontDouble32Auto();
        values.Iout               = vb.vbPopFrontDouble32Auto();
        values.Ipv                = vb.vbPopFrontDouble32Auto();
        values.Vin                = vb.vbPopFrontDouble32Auto();
        values.Vout               = vb.vbPopFrontDouble32Auto();
        values.TemperatureAmbient = vb.vbPopFrontDouble32Auto();
        values.TemperatureHeatsink= vb.vbPopFrontDouble32Auto();
        values.Power              = values.Vout*values.Iout;
        values.Eff                = vb.vbPopFrontDouble32Auto();
        values.mode               = phaseModeToStr((PhaseMode_t)(vb.vbPopFrontUint8()));
        values.fault              = phaseFaultToStr((PhaseFault_t)(vb.vbPopFrontUint8()));

        emit valuesReceived(values);
    } break;

    case COMM_GET_BMS_CELLS:{
        mTimeoutCells = 0;
        int mCellAmount;
        QVector<double> mCellVoltages;
        mCellVoltages.clear();

        mCellAmount = vb.vbPopFrontUint8();

        for(int cellValuePointer = 0; cellValuePointer < mCellAmount; cellValuePointer++){
            mCellVoltages.append(vb.vbPopFrontDouble16(1e3));
        }

        emit cellsReceived(mCellAmount,mCellVoltages);

       } break;
    case COMM_MPPT_GET_SWEEP:{
       mTimeoutSweep = 0;

       int mDatapoints = 0;
       QVector<double> vs,is;
       vs.clear();
       is.clear();

       mDatapoints = vb.vbPopFrontUint8();

       for (int i =0; i<mDatapoints;i++){
           vs.append(vb.vbPopFrontDouble16(1e2));
           is.append(vb.vbPopFrontDouble16(1e3));
       }

       emit sweepDataReceifed(vs,is);

    }break;

    case COMM_MPPT_SCOPE_RUN:{
        float sr = vb.vbPopFrontDouble32Auto();
        emit scopeSampleRateReceived(sr);
    } break;
    case COMM_MPPT_SCOPE_STEP:{
        float sr = vb.vbPopFrontDouble32Auto();
        emit scopeSampleRateReceived(sr);
    }break;

    case COMM_MPPT_SCOPE_GET_DATA:{
        int ch = vb.vbPopFrontUint8();
        int index = vb.vbPopFrontUint16();
        int size = vb.vbPopFrontUint8();
        QVector<double> data = QVector<double>(size);

        for (int i =0; i<size;i++){
            data[i] = vb.vbPopFrontDouble32Auto();
        }
        emit scopeDataReceived(ch, index, data);
    }break;


    case COMM_PRINT:
        emit printReceived(QString::fromLatin1(vb));
        break;

    case COMM_ROTOR_POSITION:
        emit rotorPosReceived(vb.vbPopFrontDouble32(1e5));
        break;

    case COMM_GET_MCCONF:
    case COMM_GET_MCCONF_DEFAULT:
        mTimeoutBMSconf = 0;
        if (mmpptConfig) {
            mmpptConfig->deSerialize(vb);
            mmpptConfig->updateDone();

            if (mCheckNextmpptConfig) {
                mCheckNextmpptConfig = false;
                emit mpptConfigCheckResult(mmpptConfig->checkDifference(&mmpptConfigLast));
            }
        }
        break;

    case COMM_SET_MCCONF:
        emit ackReceived("SEC Write OK");
        break;

    case COMM_STORE_BMS_CONF:
        if (mmpptConfig) {
            mmpptConfig->storingDone();
        }
        break;

    default:
        break;
    }
}

void Commands::getFwVersion()
{
    if (mTimeoutFwVer > 0) {
        return;
    }

    mTimeoutFwVer = mTimeoutCount;

    VByteArray vb;
    vb.vbAppendInt8(COMM_FW_VERSION);
    emitData(vb);
}

void Commands::getValues()
{
    if (mTimeoutValues > 0) {
        return;
    }

    mTimeoutValues = mTimeoutCount;

    VByteArray vb;
    vb.vbAppendInt8(COMM_GET_VALUES);
    emitData(vb);
}

void Commands::getCells()
{
    if (mTimeoutCells > 0) {
        return;
    }

    mTimeoutCells = mTimeoutCount;

    VByteArray vb;
    vb.vbAppendInt8(COMM_GET_BMS_CELLS);
    emitData(vb);
}

void Commands::getSweep(double start, double end){
    //if (mTimeoutSweep > 0){
     //   return;
    //}

    mTimeoutSweep = mTimeoutCount;

    VByteArray vb;
    vb.append(COMM_MPPT_GET_SWEEP);
    vb.vbAppendDouble16(start,1e2);
    vb.vbAppendDouble16(end,1e2);
    emitData(vb);
}

void Commands::ScopeRun(int samples, int pretrigger, int div, int ch1source, int ch2source){
    mTimeoutScope = mTimeoutCount;

    VByteArray vb;
    vb.append(COMM_MPPT_SCOPE_RUN);
    vb.vbAppendUint16(samples);
    vb.vbAppendUint16(pretrigger);
    vb.vbAppendUint8(div);
    vb.vbAppendUint8(ch1source);
    vb.vbAppendUint8(ch2source);
    emitData(vb);
}

void Commands::ScopeCurrentStep(int samples, int pretrigger, int div, int ch1source, int ch2source, float I0, float I1){
    mTimeoutScope = mTimeoutCount;

    VByteArray vb;
    vb.append(COMM_MPPT_SCOPE_STEP);
    vb.vbAppendUint16(samples);
    vb.vbAppendUint16(pretrigger);
    vb.vbAppendUint8(div);
    vb.vbAppendUint8(ch1source);
    vb.vbAppendUint8(ch2source);
    vb.vbAppendDouble32Auto(I0);
    vb.vbAppendDouble32Auto(I1);

    emitData(vb);
}
void Commands::ScopeGetData(){
    mTimeoutScope = mTimeoutCount;

    VByteArray vb;
    vb.append(COMM_MPPT_SCOPE_GET_DATA);
    emitData(vb);
}

void Commands::sendTerminalCmd(QString cmd)
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_TERMINAL_CMD);
    vb.append(cmd.toLatin1());
    emitData(vb);
}

void Commands::setDetect(disp_pos_mode mode)
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_SET_DETECT);
    vb.vbAppendInt8(mode);
    emitData(vb);
}

void Commands::samplePrint(debug_sampling_mode mode, int sample_len, int dec)
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_SAMPLE_PRINT);
    vb.vbAppendInt8(mode);
    vb.vbAppendUint16(sample_len);
    vb.vbAppendUint8(dec);
    emitData(vb);
}

void Commands::getBMSconf()
{
    if (mTimeoutBMSconf > 0) {
        return;
    }

    mTimeoutBMSconf = mTimeoutCount;


    mCheckNextmpptConfig = false;
    VByteArray vb;
    vb.vbAppendInt8(COMM_GET_MCCONF);
    emitData(vb);
}

void Commands::getBMSconfDefault()
{
    if (mTimeoutBMSconf > 0) {
        return;
    }

    mTimeoutBMSconf = mTimeoutCount;

    mCheckNextmpptConfig = false;
    VByteArray vb;
    vb.vbAppendInt8(COMM_GET_MCCONF_DEFAULT);
    emitData(vb);
}

void Commands::setBMSconf(bool check)
{
    if (mmpptConfig) {
        mmpptConfigLast = *mmpptConfig;
        VByteArray vb;
        vb.vbAppendInt8(COMM_SET_MCCONF);
        mmpptConfig->serialize(vb);
        emitData(vb);

        if (check) {
            checkmpptConfig();
        }
    }
}

void Commands::reboot()
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_REBOOT);
    emitData(vb);
}

void Commands::sendAlive()
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_ALIVE);
    emitData(vb);
}

void Commands::pairNrf(int ms)
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_NRF_START_PAIRING);
    vb.vbAppendInt32(ms);
    emitData(vb);
}

void Commands::timerSlot()
{
    if (mFirmwareIsUploading) {
        if (mFirmwareTimer) {
            mFirmwareTimer--;
        } else {
            firmwareUploadUpdate(true);
        }
    }

    if (mTimeoutFwVer > 0) mTimeoutFwVer--;
    if (mTimeoutBMSconf > 0) mTimeoutBMSconf--;
    if (mTimeoutValues > 0) mTimeoutValues--;
    if (mTimeoutCells > 0) mTimeoutCells--;
}

void Commands::emitData(QByteArray data)
{
    // Only allow firmware commands in limited mode
    if (mIsLimitedMode && data.at(0) > COMM_WRITE_NEW_APP_DATA) {
        return;
    }

    if (mSendCan) {
        data.prepend((char)mCanId);
        data.prepend((char)COMM_FORWARD_CAN);
    }

    emit dataToSend(data);
}

void Commands::firmwareUploadUpdate(bool isTimeout)
{
    if (!mFirmwareIsUploading) {
        return;
    }

    const int app_packet_size = 192;
    const int retries = 5;
    const int timeout = 350;

    if (mFirmwareState == 0) {
        mFirmwareUploadStatus = "Buffer Erase";
        if (isTimeout) {
            // Erase timed out, abort.
            mFirmwareIsUploading = false;
            mFimwarePtr = 0;
            mFirmwareUploadStatus = "Buffer Erase Timeout";
        } else {
            mFirmwareState++;
            mFirmwareRetries = retries;
            mFirmwareTimer = timeout;
            firmwareUploadUpdate(true);
        }
    } else if (mFirmwareState == 1) {
        mFirmwareUploadStatus = "CRC/Size Write";
        if (isTimeout) {
            if (mFirmwareRetries > 0) {
                mFirmwareRetries--;
                mFirmwareTimer = timeout;
            } else {
                mFirmwareIsUploading = false;
                mFimwarePtr = 0;
                mFirmwareState = 0;
                mFirmwareUploadStatus = "CRC/Size Write Timeout";
                return;
            }

            quint16 crc = Packet::crc16((const unsigned char*)mNewFirmware.constData(), mNewFirmware.size());

            VByteArray vb;
            vb.append((char)COMM_WRITE_NEW_APP_DATA);
            vb.vbAppendUint32(0);
            vb.vbAppendUint32(mNewFirmware.size());
            vb.vbAppendUint16(crc);
            vb.vbAppendUint16(0); //Append extra zero to keep offset aligned with 8 bytes.
            emitData(vb);
        } else {
            mFirmwareState++;
            mFirmwareRetries = retries;
            mFirmwareTimer = timeout;
            firmwareUploadUpdate(true);
        }
    } else if (mFirmwareState == 2) {
        mFirmwareUploadStatus = "FW Data Write";
        if (isTimeout) {
            if (mFirmwareRetries > 0) {
                mFirmwareRetries--;
                mFirmwareTimer = timeout;
            } else {
                mFirmwareIsUploading = false;
                mFimwarePtr = 0;
                mFirmwareState = 0;
                mFirmwareUploadStatus = "FW Data Write Timeout";
                return;
            }

            int fw_size_left = mNewFirmware.size() - mFimwarePtr;
            int send_size = fw_size_left > app_packet_size ? app_packet_size : fw_size_left;

            VByteArray vb;

            if (mFirmwareIsBootloader) {
                vb.append((char)COMM_WRITE_NEW_APP_DATA);
                vb.vbAppendUint32(mFimwarePtr + (1024 * 2 * 50));
            } else {
                vb.append((char)COMM_WRITE_NEW_APP_DATA);
                vb.vbAppendUint32(mFimwarePtr + 8);
            }

            vb.append(mNewFirmware.mid(mFimwarePtr, send_size));
            emitData(vb);
        } else {
            mFirmwareRetries = retries;
            mFirmwareTimer = timeout;
            mFimwarePtr += app_packet_size;

            if (mFimwarePtr >= mNewFirmware.size()) {
                mFirmwareIsUploading = false;
                mFimwarePtr = 0;
                mFirmwareState = 0;
                mFirmwareUploadStatus = "FW Upload Done";

                // Upload done. Enter bootloader!
                if (!mFirmwareIsBootloader) {
                    QByteArray buffer;
                    buffer.append((char)COMM_JUMP_TO_BOOTLOADER);
                    emitData(buffer);
                }
            } else {
                firmwareUploadUpdate(true);
            }
        }
    }
}

QString Commands::phaseFaultToStr(PhaseFault_t mode)
{

    switch (mode) {
    case Converter_OK:                 return "OK";
    case Converter_ConfigError:        return "Configuration Error";
    case Converter_InputOverVolt:      return "Input Over Voltage";
    case Converter_OutputOverVolt:     return "Output Over Voltage";
    case Converter_OutputOverCurrent:  return "Output Over Current";
    case Converter_InputOverCurrent:   return "Input Over Current";
    case Converter_InputUnderCurrent:  return "Input Under Current";

    case Converter_PhaseOverCurrent:   return "Phase Over Current";

    case Converter_Fault:              return "Fault";
    default: return "Unknown fault";
    }
}

QString Commands::phaseModeToStr(PhaseMode_t mode)
{

    switch (mode) {
    case PhaseMode_CIV:             return "Constant Input Voltage";
    case PhaseMode_CIC:             return "Constant input Current";
    case PhaseMode_MinInputCurrent: return "Minimal Input Current";
    case PhaseMode_COV:             return "Constant Output Voltage";
    case PhaseMode_COC:             return "Constant Output Current";
    case PhaseMode_TD:             return "Teperature de-rating";
    case PhaseMode_Fault:           return "Fault";

    default: return "Unknown Mode";
    }
}

void Commands::setmpptConfig(ConfigParams *mpptConfig)
{
    mmpptConfig = mpptConfig;
    connect(mmpptConfig, SIGNAL(updateRequested()), this, SLOT(getBMSconf()));
    connect(mmpptConfig, SIGNAL(updateRequestDefault()), this, SLOT(getBMSconfDefault()));
}

void Commands::startFirmwareUpload(QByteArray &newFirmware, bool isBootloader)
{
    mFirmwareIsBootloader = isBootloader;
    mFirmwareIsUploading = true;
    mFirmwareState = mFirmwareIsBootloader ? 2 : 0;
    mFimwarePtr = 0;
    mFirmwareTimer = 500;
    mFirmwareRetries = 5;
    mNewFirmware.clear();
    mNewFirmware.append(newFirmware);
    mFirmwareUploadStatus = "Buffer Erase";

    if (mFirmwareIsBootloader) {
        firmwareUploadUpdate(true);
    } else {
        VByteArray vb;
        vb.vbAppendInt8(COMM_ERASE_NEW_APP);
        vb.vbAppendUint32(mNewFirmware.size());
        emitData(vb);
    }
}

double Commands::getFirmwareUploadProgress()
{
    if (mFirmwareIsUploading) {
        return (double)mFimwarePtr / (double)mNewFirmware.size();
    } else {
        return -1.0;
    }
}

QString Commands::getFirmwareUploadStatus()
{
    return mFirmwareUploadStatus;
}

void Commands::cancelFirmwareUpload()
{
    if (mFirmwareIsUploading) {
        mFirmwareIsUploading = false;
        mFimwarePtr = 0;
        mFirmwareState = 0;
        mFirmwareUploadStatus = "Cancelled";
    }
}

void Commands::checkmpptConfig()
{
    mCheckNextmpptConfig = true;
    VByteArray vb;
    vb.vbAppendInt8(COMM_GET_MCCONF);
    emitData(vb);
}

void Commands::storempptConfig()
{
    VByteArray vb;
    vb.vbAppendInt8(COMM_STORE_BMS_CONF);
    emitData(vb);
}
