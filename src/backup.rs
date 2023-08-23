use anyhow::Context;
use std::{
    fs::File,
    io::{BufReader, BufWriter, IsTerminal},
    path::Path,
};
use tentris::metall::MetallManager;
use tracing::instrument;

#[instrument(err)]
pub fn backup(datastore_path: &Path) -> anyhow::Result<()> {
    tracing::info!("Backing up");

    if !MetallManager::is_consistent(datastore_path) {
        anyhow::bail!("Datastore is inconsistent");
    }

    if std::io::stdout().is_terminal() {
        anyhow::bail!("Refusing to dump binary data to terminal!");
    }

    let workdir = tempdir::TempDir::new("tentris_backup").context("Unable to create temporary directory")?;
    let snapshot_path = workdir.path().join("snapshot");
    let archive_path = workdir.path().join("archive");

    {
        tracing::info!("Snapshotting");
        let metall_manager = MetallManager::open(datastore_path).context("Failed to open datastore")?;

        if let Err(_) = metall_manager.snapshot(&snapshot_path) {
            anyhow::bail!("Unable to create snapshot for exporting");
        }
    }

    {
        tracing::info!("Archiving");
        let archive = BufWriter::new(File::create(&archive_path).context("Unable to open output file for writing")?);

        let mut tar_b = tar::Builder::new(archive);
        tar_b
            .append_dir_all(".", &snapshot_path)
            .context("Unable to add datastore to archive")?;

        tar_b.finish().context("Unable to write archive")?
    }

    {
        tracing::info!("Compressing");
        let archive = BufReader::new(File::open(&archive_path).context("Unable to open archive file for reading")?);

        let output = std::io::stdout().lock();
        zstd::stream::copy_encode(archive, output, 3).context("Unable to write to output file")?;
    }

    tracing::info!("Backup complete");
    Ok(())
}
