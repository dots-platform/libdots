mod ffi;

pub mod env;
pub mod msg;

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

pub type DotsResult<T> = Result<T, DotsError>;
