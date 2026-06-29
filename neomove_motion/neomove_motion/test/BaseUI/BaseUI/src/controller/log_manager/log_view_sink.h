// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/15 17:33

#ifndef BASE_UI_SRC_CONTROLLER_LOG_MANAGER_LOG_VIEW_SINK_H_
#define BASE_UI_SRC_CONTROLLER_LOG_MANAGER_LOG_VIEW_SINK_H_

#include <algorithm_process/alg_process_log_callback.h>
#include <glog/glog_helper.h>
#include <singleton.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

typedef std::function<void(const std::string& messgae, int type)> EventLogFun;
using EventLogFunId = std::size_t;

class LogViewSink : public google::LogSink, public AlgProcessLogCallback {
  SINGLETON(LogViewSink);

 public:
  LogViewSink(){};
  ~LogViewSink(){};
  void send(google::LogSeverity severity, const char* /*full_filename*/,
            const char* base_filename, int line,
            const google::LogMessageTime& time, const char* message,
            std::size_t message_len)
      override {  // severity::0为INFO，1为WARNING,2为ERROR,6为OPERATE
    if (severity > google::GLOG_INFO) {
      DispatchEventLog(std::string(message, message_len), severity);
    }
  }
  // 收到一条算法进程的日志，所有module的
  void OnAlgProcessLog(const std::string& mod_id, int log_level,
                       const std::string& log_msg,
                       const std::string& pure_msg) override {
    if (log_level > google::GLOG_INFO) {
      DispatchEventLog(pure_msg, log_level);
    }
  }
  EventLogFunId AddEventLogFun(EventLogFun fun) {
    std::lock_guard<std::mutex> lock(event_log_mutex_);
    EventLogFunId id = next_event_log_fun_id_++;
    event_log_funs_.push_back(std::make_pair(id, std::move(fun)));
    return id;
  };
  void RemoveEventLogFun(EventLogFunId id) {
    std::lock_guard<std::mutex> lock(event_log_mutex_);
    event_log_funs_.erase(
        std::remove_if(event_log_funs_.begin(), event_log_funs_.end(),
                       [id](const auto& item) { return item.first == id; }),
        event_log_funs_.end());
  }
  void ShowOperate(std::string messgae) {
    DispatchEventLog(messgae, 6);
  }

 private:
  void DispatchEventLog(const std::string& messgae, int type) {
    std::vector<EventLogFun> funs;
    {
      std::lock_guard<std::mutex> lock(event_log_mutex_);
      funs.reserve(event_log_funs_.size());
      for (const auto& item : event_log_funs_) {
        funs.push_back(item.second);
      }
    }
    for (auto& fun : funs) {
      if (fun) {
        fun(messgae, type);
      }
    }
  }

 private:
  std::mutex event_log_mutex_;
  EventLogFunId next_event_log_fun_id_ = 1;
  std::vector<std::pair<EventLogFunId, EventLogFun>> event_log_funs_;
};
using LogViewSinkSinglton = yotta::Singleton<LogViewSink>;

// 专门显示操作日志的,等级设为6
#define LOG_OPERATE(messgae) \
  LOG(INFO) << messgae;      \
  LogViewSinkSinglton::GetInstance()->ShowOperate(messgae);

#endif  // BASE_UI_SRC_CONTROLLER_LOG_MANAGER_LOG_VIEW_SINK_H_
