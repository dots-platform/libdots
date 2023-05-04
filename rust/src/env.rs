use crate::*;
use crate::ffi;

pub struct Env {}

pub fn init() -> DotsResult<Env> {
    let ret = unsafe { ffi::dots_init() };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(Env {})
}

impl Drop for Env {
    fn drop(&mut self) {
        unsafe {
            ffi::dots_finalize();
        }
    }
}

impl Env {
    pub fn get_world_rank(&self) -> usize {
        unsafe { ffi::dots_get_world_rank() }
    }

    pub fn get_world_size(&self) -> usize {
        unsafe { ffi::dots_get_world_size() }
    }
}
