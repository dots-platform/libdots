use std::ffi::c_void;
use std::mem::{self,MaybeUninit};
use std::ptr;

use crate::*;
use crate::ffi;
use crate::request::Request;

pub fn send(buf: &[u8], recipient: usize, tag: i32) -> DotsResult<()> {
    let ret = unsafe { ffi::dots_msg_send(ptr::null(), buf.as_ptr() as *const c_void, buf.len(), recipient, tag) };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(())
}

pub fn recv(buf: &mut [u8], recipient: usize, tag: i32) -> DotsResult<usize> {
    let bytes_received = unsafe {
        let mut bytes_received = MaybeUninit::<usize>::uninit();
        let ret = ffi::dots_msg_recv(ptr::null(), buf.as_ptr() as *mut c_void, buf.len(), recipient, tag, mem::transmute(&mut bytes_received));
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
        }
        bytes_received.assume_init()
    };

    Ok(bytes_received)
}

impl Request {
    pub fn send(&self, buf: &[u8], recipient: usize, tag: i32) -> DotsResult<()> {
        let ret = unsafe { ffi::dots_msg_send(&self.ffi, buf.as_ptr() as *const c_void, buf.len(), recipient, tag) };
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
        }

        Ok(())
    }

    pub fn recv(&self, buf: &mut [u8], recipient: usize, tag: i32) -> DotsResult<usize> {
        let bytes_received = unsafe {
            let mut bytes_received = MaybeUninit::<usize>::uninit();
            let ret = ffi::dots_msg_recv(&self.ffi, buf.as_ptr() as *mut c_void, buf.len(), recipient, tag, mem::transmute(&mut bytes_received));
            if ret != 0 {
                return Err(DotsError::from_ret(ret));
            }
            bytes_received.assume_init()
        };

        Ok(bytes_received)
    }
}
