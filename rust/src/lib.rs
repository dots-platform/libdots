mod ffi;

pub mod env;
pub mod msg;

use std::error::Error;
use std::fmt;

#[derive(Debug)]
pub enum DotsError {
    // Generic error used for invalid return value.
    Unknown(i32),

    Libc,
    Interface,
    Invalid,
    Internal,
}

impl DotsError {
    fn from_ret(ret: i32) -> DotsError {
        match ret {
            -1 => DotsError::Libc,
            -2 => DotsError::Interface,
            -3 => DotsError::Invalid,
            -4 => DotsError::Internal,
            _ => DotsError::Unknown(ret),
        }
    }
}

impl fmt::Display for DotsError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            DotsError::Libc => String::from("libdots libc error"),
            DotsError::Interface => String::from("libdots interface error"),
            DotsError::Invalid => String::from("libdots invalid argument error"),
            DotsError::Internal => String::from("libdots internal error"),
            DotsError::Unknown(ret) => format!("libdots unknown error: {}", ret),
        };
        f.write_str(s.as_str())
    }
}

impl Error for DotsError {}

pub type DotsResult<T> = Result<T, DotsError>;
