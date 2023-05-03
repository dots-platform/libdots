use crate::*;
use crate::ffi;
use crate::request::Request;

impl Request {
    pub fn output(&self, data: &[u8]) -> DotsResult<()> {
        let ret = unsafe { ffi::dots_output(&self.ffi, data.as_ptr(), data.len()) };
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
        }

        Ok(())
    }
}
