use std::ffi::c_void;
use std::mem::MaybeUninit;

use crate::*;
use crate::ffi;

pub fn send(buf: &[u8], recipient: usize, tag: i32) -> DotsResult<()> {
    unsafe {
        let ret = ffi::dots_msg_send(buf.as_ptr() as *const c_void, buf.len(), recipient, tag);
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
        }
    }

    Ok(())
}

pub fn recv(buf: &mut [u8], recipient: usize, tag: i32) -> DotsResult<usize> {
    let mut bytes_received: usize;
    unsafe {
        bytes_received = MaybeUninit::uninit().assume_init();
        let ret = ffi::dots_msg_recv(buf.as_ptr() as *mut c_void, buf.len(), recipient, tag, &mut bytes_received);
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
        }
    }

    Ok(bytes_received)
}
