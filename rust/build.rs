extern crate bindgen;

use std::env;
use std::path::PathBuf;
use std::process::{Command, Stdio};

use bindgen::CargoCallbacks;
use fs_extra::dir;

fn main() {
    let lib_path = PathBuf::from(".").canonicalize().unwrap().join("libdots");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let out_lib_path = out_path.join("libdots");

    let mut copy_options = dir::CopyOptions::new();
    copy_options.skip_exist = true;
    dir::copy(&lib_path, &out_path, &copy_options).unwrap();

    println!("cargo:rustc-link-search={}", out_lib_path.to_str().unwrap());
    println!("cargo:rustc-link-lib=static=dots");
    println!("cargo:rerun-if-changed={}", lib_path.join("include").join("dots.h").to_str().unwrap());

    if !Command::new("make")
        .arg("-C")
        .arg(out_lib_path.to_str().unwrap())
        .arg("libdots.a")
        .stderr(Stdio::inherit())
        .output()
        .unwrap()
        .status
        .success()
    {
        panic!("Failed to compile libdots.a");
    }

    let bindings = bindgen::Builder::default()
        .header(lib_path.join("include").join("dots.h").to_str().unwrap())
        .clang_arg(format!("-I{}", lib_path.join("include").to_str().unwrap()))
        .parse_callbacks(Box::new(CargoCallbacks))
        .generate()
        .unwrap();

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .unwrap();
}
