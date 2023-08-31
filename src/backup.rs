use anyhow::Context;
use std::{io::IsTerminal, path::Path};
use tentris::metall::MetallManager;

pub fn backup(datastore_path: &Path) -> anyhow::Result<()> {
    if std::io::stdout().is_terminal() {
        anyhow::bail!("Refusing to dump binary data to terminal");
    }

    if !datastore_path.exists() {
        anyhow::bail!("No datastore found at {}", datastore_path.display());
    }

    if !MetallManager::is_consistent(datastore_path) {
        anyhow::bail!("Datastore at {} is inconsistent", datastore_path.display());
    }

    let workdir = datastore_path
        .parent()
        .context("Unable to get parent of datastore path")?;
    let snapshot_path =
        tempdir::TempDir::new_in(workdir, ".tentris_backup").context("Unable to create temporary directory")?;

    tracing::info!("Starting backup");

    {
        tracing::info!("Snapshotting");
        let metall_manager = MetallManager::open(datastore_path).context("Failed to open datastore")?;

        if metall_manager.snapshot(&snapshot_path).is_err() {
            anyhow::bail!("Unable to create snapshot for exporting");
        }
    }

    tracing::info!("Writing backup");

    std::thread::scope(move |s| -> anyhow::Result<()> {
        let (rx, tx) = pipe::pipe_buffered();

        let archive = s.spawn(move || {
            let mut tar_b = tar::Builder::new(tx);
            tar_b
                .append_dir_all(".", &snapshot_path)
                .context("Unable to add datastore to archive")?;

            tar_b.finish().context("Unable to write archive")?;
            Ok(())
        });

        let compress = s.spawn(move || {
            let output = std::io::stdout().lock();
            zstd::stream::copy_encode(rx, output, 3).context("Unable to write to output file")?;
            Ok(())
        });

        [archive.join().unwrap(), compress.join().unwrap()]
            .into_iter()
            .collect()
    })?;

    tracing::info!("Backup complete");
    Ok(())
}
