/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>

namespace acpi {

/**
 * @brief ACPI 驱动
 *
 * 提供 ACPI 表结构定义和基本的表解析能力。
 *
 * @see https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf
 */
class Acpi {
 public:
  /// @name 构造/析构函数
  /// @{
  /**
   * @brief 构造函数
   * @param rsdp RSDP 地址
   */
  explicit Acpi(uint64_t rsdp) : rsdp_addr_(rsdp) {}
  Acpi() = default;
  Acpi(const Acpi&) = delete;
  Acpi(Acpi&&) = default;
  auto operator=(const Acpi&) -> Acpi& = delete;
  auto operator=(Acpi&&) -> Acpi& = default;
  ~Acpi() = default;
  /// @}

 private:
  /**
   * @brief Generic Address Structure
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.3.2
   */
  struct [[gnu::packed]] GenericAddressStructure {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t access_size;
    uint64_t address;
  };

  /**
   * @brief Root System Description Pointer (RSDP) Structure
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.5.3
   */
  struct [[gnu::packed]] Rsdp {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
  };

  /**
   * @brief System Description Table Header
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.6
   */
  struct [[gnu::packed]] DescriptionHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
  };

  /**
   * @brief Root System Description Table (RSDT)
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.7
   */
  struct [[gnu::packed]] Rsdt {
    DescriptionHeader header;
    uint32_t* entry;
  };

  /**
   * @brief Extended System Description Table (XSDT)
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.8
   */
  struct [[gnu::packed]] Xsdt {
    DescriptionHeader header;
    uint64_t* entry;
  };

  /**
   * @brief Fixed ACPI Description Table (FADT)
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.9
   */
  struct [[gnu::packed]] Fadt {
    DescriptionHeader header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    uint8_t pm2_cnt_len;
    uint8_t pm_tmr_len;
    uint8_t gpe0_blk_len;
    uint8_t gpe1_blk_len;
    uint8_t gpe1_base;
    uint8_t cst_cnt;
    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alrm;
    uint8_t mon_alrm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t reserved2;
    uint32_t flags;
    GenericAddressStructure reset_reg;
    uint8_t reset_value;
    uint16_t arm_boot_arch;
    uint8_t fadt_minor_version;
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    GenericAddressStructure x_pm1a_evt_blk;
    GenericAddressStructure x_pm1b_evt_blk;
    GenericAddressStructure x_pm1a_cnt_blk;
    GenericAddressStructure x_pm1b_cnt_blk;
    GenericAddressStructure x_pm2_cnt_blk;
    GenericAddressStructure x_pm_tmr_blk;
    GenericAddressStructure x_gpe0_blk;
    GenericAddressStructure x_gpe1_blk;
    GenericAddressStructure sleep_control_reg;
    GenericAddressStructure sleep_status_reg;
    uint64_t hypervisor_vendor_id;
  };

  /**
   * @brief Differentiated System Description Table (DSDT)
   * @see ACPI_Spec_6_5_Aug29.pdf#5.2.11.1
   */
  struct [[gnu::packed]] Dsdt {
    DescriptionHeader header;
    uint8_t* definition_block;
  };

  uint64_t rsdp_addr_{0};
};

}  // namespace acpi
