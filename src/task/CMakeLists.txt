# Copyright The SimpleKernel Contributors

ADD_LIBRARY (task INTERFACE)

TARGET_INCLUDE_DIRECTORIES (task INTERFACE include)

TARGET_SOURCES (
    task
    INTERFACE task_control_block.cpp
              schedule.cpp
              exit.cpp
              tick_update.cpp
              sleep.cpp
              block.cpp
              wakeup.cpp
              clone.cpp
              wait.cpp
              task_manager.cpp
              mutex.cpp
              signal.cpp)
