#include "qt_async_serial.h"

#include <glog/glog_helper.h>

#include <QStringList>
#include <QTime>

using namespace std::placeholders;

QAsyncSerial::QAsyncSerial() {}

QAsyncSerial::QAsyncSerial(QString devname, unsigned int baudrate) {
  if (!Open(devname, baudrate)) {
    LOG(WARNING) << "打开串口" << devname.toStdString() << "失败";
    return;
  }
}

bool QAsyncSerial::Open(QString devname, unsigned int baudrate) {
  if (!serial_port_helper_) {
    if (!serial_port_helper_.InitDll(L"async_serial_port.dll")) {
      LOG(WARNING) << "serial_port_helper初始化失败，可能缺少dll";
      return false;
    }
  }
  serial_port_helper_->Init(
      serial_port::Parity::kPAR_NONE, serial_port::DataBits::kDATA_8,
      serial_port::StopBits::kSTOP_2, serial_port::FlowControl::kOff);

  err_ =
      serial_port_helper_->Open(devname.toStdString().c_str(), baudrate, this);
  if (err_ != 0) {
    LOG(WARNING) << "打开串口：" << devname.toStdString() << "失败！结果："
                 << err_;
  }

  return serial_port_helper_->IsOpen();
}

void QAsyncSerial::Close() {
  if (serial_port_helper_) {
    serial_port_helper_->Close();
  }
}

bool QAsyncSerial::IsOpen() {
  if (serial_port_helper_) {
    return serial_port_helper_->IsOpen();
  }

  return false;
}

void QAsyncSerial::Stop() {}

bool QAsyncSerial::ErrorStatus() { return err_ != 0; }

bool QAsyncSerial::Write(QByteArray data) {
  if (!serial_port_helper_) {
    LOG(WARNING) << "串口为空指针";
    return false;
  }

  if (!serial_port_helper_->IsOpen()) {
    LOG(WARNING) << "串口未打开";
    return false;
  }

  serial_port_helper_->AsyncWrite(data.constData(), data.size());
  // LOG(INFO) << "发送指令" << data.constData();
  return true;
}

QAsyncSerial::~QAsyncSerial() { Close(); }

void YOTTA_API_CALL QAsyncSerial::OnReceiveData(const char *data, size_t size) {
  QString received_data = QString::fromLatin1(data, static_cast<int>(size));
  qint64 receive_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
  // LOG(INFO) << "收到指令：[" << receive_time << "]"
  //         << received_data;
  emit LineReceived(received_data, receive_time);
}

void YOTTA_API_CALL QAsyncSerial::OnWriteFinish(unsigned int task_id, int err,
                                                const char *err_msg) {
  write_finish_count_++;
  if (err != 0) {
    LOG(ERROR) << "发送数据失败，错误码：" << err << "，错误信息：" << err_msg;
  }
}

void YOTTA_API_CALL QAsyncSerial::OnError(int err, const char *err_msg) {
  err_ = err;
  err_msg_ = err_msg;
  LOG(ERROR) << "串口出错，错误码：" << err << "，错误信息：" << err_msg;
}
