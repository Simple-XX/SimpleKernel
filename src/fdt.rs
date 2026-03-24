use crate::error::{ErrorCode, KResult};
use core::marker::PhantomData;

#[derive(Debug)]
pub struct KernelFdt<'a> {
    fdt_addr: usize,
    _marker: PhantomData<&'a [u8]>,
}

macro_rules! parse_fdt {
    ($addr:expr) => {{
        // SAFETY: fdt_addr was validated in KernelFdt::new()
        unsafe { fdt::Fdt::from_ptr_unaligned_fallible($addr as *const u8) }
            .map_err(|_| ErrorCode::FdtInvalidHeader)
    }};
}

impl<'a> KernelFdt<'a> {
    pub fn new(fdt_addr: usize) -> KResult<Self> {
        // SAFETY: fdt_addr is validated by the caller (passed from bootloader via DTB)
        unsafe { fdt::Fdt::from_ptr_unaligned(fdt_addr as *const u8) }
            .map_err(|_| ErrorCode::FdtInvalidHeader)?;
        Ok(Self {
            fdt_addr,
            _marker: PhantomData,
        })
    }

    pub fn core_count(&self) -> KResult<usize> {
        let fdt = parse_fdt!(self.fdt_addr)?;
        let root = fdt.root().map_err(|_| ErrorCode::FdtParseFailed)?;
        let cpus = root.cpus().map_err(|_| ErrorCode::FdtNodeNotFound)?;
        let iter = cpus.iter().map_err(|_| ErrorCode::FdtParseFailed)?;
        let count = iter.filter_map(|c| c.ok()).count();
        if count == 0 {
            return Err(ErrorCode::FdtNodeNotFound);
        }
        Ok(count)
    }

    pub fn memory(&self) -> KResult<(u64, usize)> {
        let fdt = parse_fdt!(self.fdt_addr)?;
        let root = fdt.root().map_err(|_| ErrorCode::FdtParseFailed)?;
        let memory = root.memory().map_err(|_| ErrorCode::FdtNodeNotFound)?;
        let mut regions = memory
            .reg()
            .map_err(|_| ErrorCode::FdtPropertyNotFound)?
            .iter::<u64, usize>();
        let region = regions
            .next()
            .ok_or(ErrorCode::FdtNodeNotFound)?
            .map_err(|_| ErrorCode::FdtParseFailed)?;
        Ok((region.address, region.len))
    }

    pub fn timebase_frequency(&self) -> KResult<u32> {
        let fdt = parse_fdt!(self.fdt_addr)?;
        let cpus = fdt
            .find_node("/cpus")
            .map_err(|_| ErrorCode::FdtParseFailed)?
            .ok_or(ErrorCode::FdtNodeNotFound)?;
        let prop = cpus
            .raw_property("timebase-frequency")
            .map_err(|_| ErrorCode::FdtParseFailed)?
            .ok_or(ErrorCode::FdtPropertyNotFound)?;
        let bytes: [u8; 4] = prop
            .value
            .try_into()
            .map_err(|_| ErrorCode::FdtInvalidPropertySize)?;
        Ok(u32::from_be_bytes(bytes))
    }

    #[must_use]
    pub fn node_count(&self) -> usize {
        let Ok(fdt) = parse_fdt!(self.fdt_addr) else {
            return 0;
        };
        let Ok(nodes) = fdt.all_nodes() else { return 0 };
        nodes.filter_map(|n| n.ok()).count()
    }
}
