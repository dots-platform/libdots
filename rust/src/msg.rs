use std::ffi::c_void;
use std::mem::MaybeUninit;

use crate::ffi as ffi;

pub fn send(buf: &Vec<u8>, recipient: usize, tag: i32) -> Result<(), &'static str> {
    unsafe {
        if ffi::dots_msg_send(buf.as_ptr() as *const c_void, buf.len(), recipient, tag) != 0 {
            return Err("dots_msg_send");
        }
    }

    Ok(())
}

pub fn recv(buf: &mut Vec<u8>, recipient: usize, tag: i32) -> Result<usize, &'static str> {
    unsafe {
        let mut bytes_received: usize = MaybeUninit::uninit().assume_init();
        if ffi::dots_msg_recv(buf.as_ptr() as *mut c_void, buf.len(), recipient, tag, &mut bytes_received) != 0 {
            return Err("dots_msg_send");
        }

        Ok(bytes_received)
    }
}
