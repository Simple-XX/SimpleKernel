#![allow(dead_code)]

use core::fmt;

const BUF_SIZE: usize = 256;

pub struct FmtBuf {
    buf: [u8; BUF_SIZE],
    pub pos: usize,
    truncated: bool,
}

impl FmtBuf {
    pub const fn new() -> Self {
        Self {
            buf: [0; BUF_SIZE],
            pos: 0,
            truncated: false,
        }
    }

    pub fn as_str(&self) -> &str {
        core::str::from_utf8(&self.buf[..self.pos]).unwrap_or("")
    }

    pub fn is_truncated(&self) -> bool {
        self.truncated
    }
}

impl fmt::Write for FmtBuf {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        let available = BUF_SIZE.saturating_sub(self.pos);
        let to_copy = available.min(bytes.len());
        if to_copy > 0 {
            self.buf[self.pos..self.pos + to_copy].copy_from_slice(&bytes[..to_copy]);
            self.pos += to_copy;
        }
        if to_copy < bytes.len() {
            self.truncated = true;
        }
        Ok(())
    }
}
