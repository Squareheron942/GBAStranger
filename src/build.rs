//! Build script to generate the rust code for the rooms. 

use serde::Deserialize;
use std::{io::{self, Write, BufWriter}, env, path::{Path, PathBuf}, ffi::OsString, fs::{self, File}};
use heck::ToShoutySnakeCase;

/// Structs to represent rooms that are included in the top of the generated rooms.rs. 
const STRUCTS: &'static [u8] = br#"

/// Representation of a room in void stranger. 
struct Room {
    /// The name of the room. Usually matches the file name. 
    name: &'static str,

    /// Name of gml file called when room is created. 
    creation_code_id: &'static str,
}

"#;

// Deserializable version of all structs put here for use with serde.
#[derive(Deserialize)]
struct Room {
    name: String, 
    creation_code_id: String,
}

fn main() -> io::Result<()> {
    // Re-run if any assets change
    println!("cargo:rerun-if-changed=assets/");

    // Generate rust code that includes all rooms from JSON. 
    let out_dir: OsString = env::var_os("OUT_DIR").unwrap();
    let out_path: PathBuf = Path::new(&out_dir).join("rooms.rs");
    let mut out_file: BufWriter<File> = BufWriter::new(File::create(&out_path).expect("created output file"));

    // Write the struct definitions to the file. 
    out_file.write_all(STRUCTS)?;
    eprintln!("wrote struct definitions to {}", out_path.display());

    // Create a list of all rooms so that we can get a constant array of all of them. 
    let mut all_rooms = Vec::new();

    // Walk through all the files in the assets/roomExport
    for entry in fs::read_dir("assets/roomExport").expect("read_dir assets/roomExport") {
        // Assume that the whole dir is .json room files. 
        // Get the path of the directory entry.
        let path: PathBuf = entry?.path();
        // Log that we're processing the current file.
        eprintln!("Processing {}...", path.display());
        // Open the file at that path and deserialize it from json. 
        let room: Room = serde_json::from_reader(File::open(&path)?)?;
        // The name for the constant should be an all caps version of the room name. 
        let const_name = room.name.to_shouty_snake_case();
        // Add reference to room name to list of all rooms.
        all_rooms.push(format!("&{const_name}"));
        // Write a const to the end of the rooms file.
        // Use `#[allow(dead_code)]` to spress warnings. 
        writeln!(&mut out_file, "#[allow(dead_code)]")?;
        writeln!(&mut out_file, r#"const {const_name}: Room = Room {{ name: "{}", creation_code_id: "{}" }};"#, room.name, room.creation_code_id)?;
        

        // Log message visible to cargo build -vv
        eprintln!("translated {} to a rust constant.", path.display());
    }

    // Add all rooms constant.
    writeln!(&mut out_file, "#[allow(dead_code)]")?;
    writeln!(&mut out_file, "const ALL_ROOMS: [&'static Room; {}] = [{}];", all_rooms.len(), all_rooms.join(", "))?;

    // Flush all changes to the file. It will be closed with the file object is dropped at the end of this function. 
    out_file.flush()?;

    // If no errors, return OK. 
    return Ok(());
}
