mod ffi;

pub mod env;
pub mod msg;

#[derive(Debug)]
pub enum DotsError {
    // Generic error used for invalid return value.
    Unknown = 0,

    Libc = -1,
    Interface = -2,
    Invalid = -3,
    Internal = -4,
}

impl DotsError {
    fn from_ret(ret: i32) -> DotsError {
        match ret {
            x if x == DotsError::Libc as i32 => DotsError::Libc,
            x if x == DotsError::Interface as i32 => DotsError::Interface,
            x if x == DotsError::Invalid as i32 => DotsError::Invalid,
            x if x == DotsError::Internal as i32 => DotsError::Internal,
            _ => DotsError::Unknown,
        }
    }
}

pub type DotsResult<T> = Result<T, DotsError>;
