/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/singleton.h>

#include "pl011/pl011_driver.hpp"

/// PL011 串口设备单例类型别名
using Pl011Singleton = etl::singleton<pl011::Pl011Device>;
