use anyhow::Context;
use std::path::Path;
use tentris::{metall::MetallManager, triplestore::TripleStore};
use tokio::{io::AsyncWriteExt, signal::unix::SignalKind};

pub fn dump(datastore_path: &Path) -> anyhow::Result<()> {
    let runtime = tokio::runtime::Builder::new_current_thread()
        .enable_io()
        .thread_name("tentris-worker")
        .build()?;

    if !MetallManager::is_consistent(datastore_path) {
        anyhow::bail!("Datastore is inconsistent");
    }

    let metall_manager = MetallManager::open(datastore_path).context("Failed to open datastore")?;

    let ts = TripleStore::open_persistent(metall_manager, crate::TRIPLESTORE_NAME)
        .context("Unable to load persisted triplestore from datastore")?;

    let output_writer = async move {
        let mut output = tokio::io::BufWriter::new(tokio::io::stdout());

        let mut iter = ts.iter();
        while let Some([s, p, o]) = iter.next() {
            let buf = format!("{s} {p} {o} .\n");
            output
                .write_all(buf.as_bytes())
                .await
                .context("Unable to write output")?;
        }

        Ok::<_, anyhow::Error>(())
    };

    runtime.block_on(async move {
        let mut sigterm = tokio::signal::unix::signal(SignalKind::terminate())
            .context("Could not create signal handler for SIGTERM")
            .unwrap();

        let mut sigint = tokio::signal::unix::signal(SignalKind::interrupt())
            .context("Could not create signal handler for SIGINT")
            .unwrap();

        tokio::select! {
            _ = sigterm.recv() => Ok(()),
            _ = sigint.recv() => Ok(()),
            res = output_writer => res,
        }
    })
}
