use crate::*;
use crate::ffi;

pub fn init() -> DotsResult<()> {
    let ret = unsafe { ffi::dots_init() };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(())
}

pub fn finalize() {
    unsafe {
        ffi::dots_finalize();
    }
}
