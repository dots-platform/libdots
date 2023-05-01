use crate::*;
use crate::ffi;

pub fn output(data: &[u8]) -> DotsResult<()> {
    let ret = unsafe { ffi::dots_output(data.as_ptr(), data.len()) };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(())
}
