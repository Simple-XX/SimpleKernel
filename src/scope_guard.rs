/// RAII cleanup guard — runs cleanup on Drop, dismiss() cancels it.
/// Pattern from Linux kernel Rust — used for init failure rollback.
#[allow(dead_code)]
pub struct ScopeGuard<F: FnOnce()> {
    cleanup: Option<F>,
}

#[allow(dead_code)]
impl<F: FnOnce()> ScopeGuard<F> {
    pub fn new(cleanup: F) -> Self {
        Self {
            cleanup: Some(cleanup),
        }
    }

    /// Cancel cleanup (call on success path).
    pub fn dismiss(mut self) {
        self.cleanup = None;
    }
}

impl<F: FnOnce()> Drop for ScopeGuard<F> {
    fn drop(&mut self) {
        if let Some(cleanup) = self.cleanup.take() {
            cleanup();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::cell::Cell;

    #[test]
    fn runs_cleanup_on_drop() {
        let ran = Cell::new(false);
        {
            let _guard = ScopeGuard::new(|| ran.set(true));
        }
        assert!(ran.get());
    }

    #[test]
    fn dismiss_prevents_cleanup() {
        let ran = Cell::new(false);
        {
            let guard = ScopeGuard::new(|| ran.set(true));
            guard.dismiss();
        }
        assert!(!ran.get());
    }
}
