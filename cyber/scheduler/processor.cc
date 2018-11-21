/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "cyber/scheduler/processor.h"

#include <sched.h>
#include <chrono>

#include "cyber/common/global_data.h"
#include "cyber/common/log.h"
#include "cyber/croutine/croutine.h"
#include "cyber/croutine/routine_context.h"
#include "cyber/scheduler/processor_context.h"
#include "cyber/scheduler/scheduler.h"

namespace apollo {
namespace cyber {
namespace scheduler {

using apollo::cyber::common::GlobalData;

Processor::Processor() {
  running_.exchange(true);
  routine_context_.reset(new RoutineContext());
  thread_ = std::thread(&Processor::Run, this);
}

Processor::~Processor() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

void Processor::SetAffinity(const std::vector<int> &cpus,
                            const std::string &affinity, int p) {
  cpu_set_t set;
  CPU_ZERO(&set);

  if (cpus.size()) {
    if (!affinity.compare("range")) {
      for (std::vector<int>::const_iterator it = cpus.begin(), e = cpus.end();
            it != e; it++) {
        CPU_SET(*it, &set);
      }
      pthread_setaffinity_np(thread_.native_handle(), sizeof(set), &set);
    } else if (!affinity.compare("1to1")) {
      CPU_SET(cpus[p], &set);
      pthread_setaffinity_np(thread_.native_handle(), sizeof(set), &set);
    }
  }
}

void Processor::SetSchedPolicy(std::string spolicy, int sched_priority) {
  struct sched_param sp;
  int policy;

  if (!spolicy.compare("SCHED_FIFO")) {
    policy = SCHED_FIFO;
  } else if (!spolicy.compare("SCHED_RR")) {
    policy = SCHED_RR;
  } else {
    return;
  }

  memset(reinterpret_cast<void *>(&sp), 0, sizeof(sp));
  sp.sched_priority = sched_priority;

  pthread_setschedparam(thread_.native_handle(), policy, &sp);
}

void Processor::Run() {
  CRoutine::SetMainContext(routine_context_);

  while (likely(running_)) {
    if (likely(context_ != nullptr)) {
      auto croutine = context_->NextRoutine();
      if (croutine) {
        croutine->Resume();
        croutine->Release();
      } else {
        context_->Wait();
      }
    } else {
      std::unique_lock<std::mutex> lk(mtx_ctx_);
      cv_ctx_.wait(lk, [this]{ return this->context_ != nullptr; });
    }
  }
}

}  // namespace scheduler
}  // namespace cyber
}  // namespace apollo
