use anyhow::Context;
use std::{
    io::{BufReader, IsTerminal},
    path::Path,
};

pub fn restore(datastore_path: &Path) -> anyhow::Result<()> {
    if std::io::stdin().is_terminal() {
        anyhow::bail!("Refusing to read binary data from terminal");
    }

    if datastore_path.exists() {
        anyhow::bail!(
            "Cannot restore to {}, destination already exists",
            datastore_path.display()
        );
    }

    tracing::info!("Restoring");

    std::thread::scope(|s| -> anyhow::Result<()> {
        let (rx, tx) = pipe::pipe_buffered();

        let uncompress = s.spawn(move || {
            let input = BufReader::new(std::io::stdin().lock());
            zstd::stream::copy_decode(input, tx)?;
            Ok(())
        });

        let unarchive = s.spawn(move || {
            let mut archive = tar::Archive::new(rx);
            archive.unpack(datastore_path).context("Unable to unpack archive")?;
            Ok(())
        });

        [uncompress.join().unwrap(), unarchive.join().unwrap()]
            .into_iter()
            .collect()
    })?;

    tracing::info!("Restore successful");
    Ok(())
}
