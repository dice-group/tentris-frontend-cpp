use anyhow::Context;
use std::{
    fs::File,
    io::{BufReader, BufWriter, IsTerminal},
    path::Path,
};

pub fn restore(datastore_path: &Path) -> anyhow::Result<()> {
    tracing::info!("Restoring");

    if std::io::stdin().is_terminal() {
        anyhow::bail!("Refusing to read binary data from terminal");
    }

    let workdir =
        tempdir::TempDir::new("tentris_restore").context("Unable to create temporary directory for writing")?;

    let uncompressed_tmp_path = workdir.path().join("uncompressed");

    {
        tracing::info!("Uncompressing");
        let input = std::io::stdin().lock();
        let f = BufReader::new(input);

        let uncompressed =
            BufWriter::new(File::create(&uncompressed_tmp_path).context("Unable to open temporary file for writing")?);

        zstd::stream::copy_decode(f, uncompressed).context("Unable to write to temporary file")?;
    }

    {
        tracing::info!("Unarchiving");
        let uncompressed =
            BufReader::new(File::open(&uncompressed_tmp_path).context("Unable to open temporary file for reading")?);

        let mut archive = tar::Archive::new(uncompressed);
        archive.unpack(datastore_path).context("Unable to unpack archive")?;
    }

    tracing::info!("Restore successful");
    Ok(())
}
