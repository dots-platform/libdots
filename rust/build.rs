extern crate bindgen;

use std::env;
use std::path::PathBuf;
use std::process::Command;

use bindgen::CargoCallbacks;

fn main() {
    let lib_path = PathBuf::from("..").canonicalize().unwrap();

    println!("cargo:rustc-link-search={}", lib_path.to_str().unwrap());
    println!("cargo:rustc-link-lib=static=dots");
    println!("cargo:rerun-if-changed={}", lib_path.join("include").join("dots.h").to_str().unwrap());

    if !Command::new("make")
        .arg("-C")
        .arg("..")
        .arg("libdots.a")
        .output()
        .unwrap()
        .status
        .success()
    {
        panic!("Failed to compile libdots.a");
    }

    let bindings = bindgen::Builder::default()
        .header("../include/dots.h")
        .clang_arg("-I../include")
        .parse_callbacks(Box::new(CargoCallbacks))
        .generate()
        .unwrap();

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap()).join("bindings.rs");
    bindings
        .write_to_file(out_path)
        .unwrap();
}
