#ifndef COUPLING_MACHINE_SRC_CONTROLLER_POWER_METER_READ_QT_ASYNC_SERIAL_QT_ASYNC_SERIAL_H_
#define COUPLING_MACHINE_SRC_CONTROLLER_POWER_METER_READ_QT_ASYNC_SERIAL_QT_ASYNC_SERIAL_H_

#include <serial_port/async_serial_port_helper.h>

#include <QObject>
#include <memory>

#include <condition_variable>
#include <mutex>


class QAsyncSerial : public QObject, public serial_port::SerialPortCallback {
  Q_OBJECT

 public:
  QAsyncSerial();

  QAsyncSerial(QString devname, unsigned int baudrate);

  // 打开串口
  bool Open(QString devname, unsigned int baudrate);

  // 关闭串口
  void Close();

  // open 返回 true
  bool IsOpen();

  // error 返回 true
  bool ErrorStatus();

  // 写入data
  bool Write(QByteArray data);

  virtual void Stop();

  ~QAsyncSerial();

 public:
  // 数据缓冲区只读，不能释放
  void YOTTA_API_CALL OnReceiveData(const char*, size_t len) override;

  // write之后的回调，第一个参数为调用write时的返回值，
  void YOTTA_API_CALL OnWriteFinish(unsigned int task_id, int err,
                                    const char* err_msg) override;
  // 出错了的回调
  void YOTTA_API_CALL OnError(int err, const char* err_msg) override;

 signals:
  // 收到数据的信号
  void LineReceived(QString data, qint64 receive_time);

 private:
  AsyncSerialPortHelper serial_port_helper_;
  int err_ = 0;
  std::string err_msg_;

  int write_finish_count_ = 0;
};

#endif  // COUPLING_MACHINE_SRC_CONTROLLER_POWER_METER_READ_QT_ASYNC_SERIAL_QT_ASYNC_SERIAL_H_
