OpenSBI SBI PMU extension support
==================================
SBI PMU extension supports allow supervisor software to configure/start/stop
any performance counter at anytime. Thus, a user can leverage full
capability of performance analysis tools such as perf if SBI PMU extension is
enabled. The OpenSBI implementation makes the following assumptions about the
hardware platform.

 * The platform must provide information about PMU event to counter mapping
via device tree or platform specific hooks. Otherwise, SBI PMU extension will
not be enabled.

 * The platforms should provide information about the PMU event selector values
that should be encoded in the expected value of MHPMEVENTx while configuring
MHPMCOUNTERx for that specific event. This can be done via a device tree or
platform specific hooks. The exact value to be written to he MHPMEVENTx is
completely depends on platform. Generic platform writes the zero-extended event_idx
as the expected value for hardware cache/generic events as suggested by the SBI
specification.

SBI PMU Device Tree Bindings
----------------------------

Platforms may choose to describe PMU event selector and event to counter mapping
values via device tree. The following sections describe the PMU DT node
bindings in details.

* **compatible** (Mandatory) - The compatible string of SBI PMU device tree node.
This DT property must have the value **riscv,pmu**.

* **riscv,event-to-mhpmevent**(Optional) - It represents an ONE-to-ONE mapping
between a PMU event and the event selector value that platform expects to be
written to the MHPMEVENTx CSR for that event. The mapping is encoded in a
table format where each row represents an event. The first column represent the
event idx where the 2nd & 3rd column represent the event selector value that
should be encoded in the expected value to be written in MHPMEVENTx.
This property shouldn't encode any raw hardware event.

* **riscv,event-to-mhpmcounters**(Optional) - It represents a MANY-to-MANY
mapping between a range of events and all the MHPMCOUNTERx in a bitmap format
that can be used to monitor these range of events. The information is encoded in
a table format where each row represents a certain range of events and
corresponding counters. The first column represents starting of the pmu event id
and 2nd column represents the end of the pmu event id. The third column
represent a bitmap of all the MHPMCOUNTERx. This property is mandatory if
riscv,event-to-mhpmevent is present. Otherwise, it can be omitted. This property
shouldn't encode any raw event.

* **riscv,raw-event-to-mhpmcounters**(Optional) - It represents an ONE-to-MANY
or MANY-to-MANY mapping between the raw event(s) and all the MHPMCOUNTERx in
a bitmap format that can be used to monitor that raw event. The encoding of the
raw events are platform specific. The information is encoded in a table format
where each row represents the specific raw event(s). The first column is a 64bit
match value where the invariant bits of range of events are set. The second
column is a 64 bit mask that will have all the variant bits of the range of
events cleared. All other bits should be set in the mask.
The third column is a 32bit value to represent bitmap of all MHPMCOUNTERx that
can monitor these set of event(s).
If a platform directly encodes each raw PMU event as a unique ID, the value of
select_mask must be 0xffffffff_ffffffff.

*Note:* A platform may choose to provide the mapping between event & counters
via platform hooks rather than the device tree.

### Example 1

```
pmu {
	compatible 			= "riscv,pmu";
	riscv,event-to-mhpmevent 		= <0x0000B  0x0000 0x0001>;
	riscv,event-to-mhpmcounters 	= <0x00001 0x00001 0x00000001>,
						  <0x00002 0x00002 0x00000004>,
						  <0x00003 0x0000A 0x00000ff8>,
						  <0x10000 0x10033 0x000ff000>;
					/* For event ID 0x0002 */
	riscv,raw-event-to-mhpmcounters = <0x0000 0x0002 0xffffffff 0xffffffff 0x00000f8>,
					/* For event ID 0-4 */
					<0x0 0x0 0xffffffff 0xfffffff0 0x00000ff0>,
					/* For event ID 0xffffffff0000000f - 0xffffffff000000ff */
					<0xffffffff 0x0 0xffffffff 0xffffff0f 0x00000ff0>;
};
```

### Example 2

```
/*
 * For HiFive Unmatched board. The encodings can be found here
 * https://sifive.cdn.prismic.io/sifive/1a82e600-1f93-4f41-b2d8-86ed8b16acba_fu740-c000-manual-v1p6.pdf
 * This example also binds standard SBI PMU hardware id's to U74 PMU event codes, U74 uses bitfield for
 * events encoding, so several U74 events can be bound to single perf id.
 * See SBI PMU hardware id's in include/sbi/sbi_ecall_interface.h
 */
pmu {
	compatible 			= "riscv,pmu";
	riscv,event-to-mhpmevent =
/* SBI_PMU_HW_CACHE_REFERENCES -> Instruction cache/ITIM busy | Data cache/DTIM busy */
				   <0x00003 0x00000000 0x1801>,
/* SBI_PMU_HW_CACHE_MISSES -> Instruction cache miss | Data cache miss or memory-mapped I/O access */
				   <0x00004 0x00000000 0x0302>,
/* SBI_PMU_HW_BRANCH_INSTRUCTIONS -> Conditional branch retired */
				   <0x00005 0x00000000 0x4000>,
/* SBI_PMU_HW_BRANCH_MISSES -> Branch direction misprediction | Branch/jump target misprediction */
				   <0x00006 0x00000000 0x6001>,
/* L1D_READ_MISS -> Data cache miss or memory-mapped I/O access */
				   <0x10001 0x00000000 0x0202>,
/* L1D_WRITE_ACCESS -> Data cache write-back */
				   <0x10002 0x00000000 0x0402>,
/* L1I_READ_ACCESS -> Instruction cache miss */
				   <0x10009 0x00000000 0x0102>,
/* LL_READ_MISS -> UTLB miss */
				   <0x10011 0x00000000 0x2002>,
/* DTLB_READ_MISS -> Data TLB miss */
				   <0x10019 0x00000000 0x1002>,
/* ITLB_READ_MISS-> Instruction TLB miss */
				   <0x10021 0x00000000 0x0802>;
	riscv,event-to-mhpmcounters = <0x00003 0x00006 0x18>,
				      <0x10001 0x10002 0x18>,
				      <0x10009 0x10009 0x18>,
				      <0x10011 0x10011 0x18>,
				      <0x10019 0x10019 0x18>,
				      <0x10021 0x10021 0x18>;
	riscv,raw-event-to-mhpmcounters = <0x0 0x0 0xffffffff 0xfc0000ff 0x18>,
					  <0x0 0x1 0xffffffff 0xfff800ff 0x18>,
					  <0x0 0x2 0xffffffff 0xffffe0ff 0x18>;
};
```
