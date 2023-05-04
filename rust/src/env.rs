use crate::*;
use crate::ffi;

pub struct EnvContext {}

pub fn init() -> DotsResult<EnvContext> {
    let ret = unsafe { ffi::dots_init() };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(EnvContext {})
}

impl Drop for EnvContext {
    fn drop(&mut self) {
        unsafe {
            ffi::dots_finalize();
        }
    }
}
