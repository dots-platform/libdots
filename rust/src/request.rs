use std::ffi::CStr;
use std::fs::File;
use std::mem::{self,MaybeUninit};
use std::os::unix::io::FromRawFd;
use std::slice;

use uuid::Uuid;

use crate::*;
use crate::ffi;

pub struct Request {
    pub id: Uuid,
    pub world_rank: usize,
    pub world_size: usize,
    pub in_files: Vec<File>,
    pub out_files: Vec<File>,
    pub func_name: String,
    pub args: Vec<Vec<u8>>,
    pub ffi: ffi::dots_request_t,
}

pub fn accept() -> DotsResult<Request> {
    let request_ffi = {
        let mut request = MaybeUninit::<ffi::dots_request_t>::uninit();
        unsafe {
            ffi::dots_request_accept(mem::transmute(request.as_mut_ptr()));
            request.assume_init()
        }
    };

    let in_files = unsafe { slice::from_raw_parts(request_ffi.in_fds, request_ffi.in_fds_len) }
        .iter()
        .map(|fd| unsafe { File::from_raw_fd(*fd) })
        .collect();

    let out_files = unsafe { slice::from_raw_parts(request_ffi.out_fds, request_ffi.out_fds_len) }
        .iter()
        .map(|fd| unsafe { File::from_raw_fd(*fd) })
        .collect();

    let func_name = unsafe { CStr::from_ptr(request_ffi.func_name) }
        .to_str()
        .expect("Function name is not UTF-8 encoded")
        .to_owned();

    let args = unsafe { slice::from_raw_parts(request_ffi.args, request_ffi.args_len) }
        .iter()
        .map(|arg_ffi| unsafe { slice::from_raw_parts(arg_ffi.ptr, arg_ffi.length) }.to_owned())
        .collect();

    let request = Request {
        id: Uuid::from_bytes(request_ffi.id),
        world_rank: request_ffi.world_rank,
        world_size: request_ffi.world_size,
        in_files,
        out_files,
        func_name,
        args,
        ffi: request_ffi,
    };
    Ok(request)
}

impl Drop for Request {
    fn drop(&mut self) {
        unsafe {
            ffi::dots_request_free(&mut self.ffi);
        }
    }
}
