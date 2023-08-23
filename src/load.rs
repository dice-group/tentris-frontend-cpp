use anyhow::Context;
use std::{io::IsTerminal, path::Path};
use tentris::{metall::MetallManager, triplestore::TripleStore};
use tracing::instrument;

#[instrument(err)]
pub fn load(datastore_path: &Path) -> anyhow::Result<()> {
    if std::io::stdin().is_terminal() {
        anyhow::bail!("Refusing to load turtle from terminal");
    }

    let metall_manager = MetallManager::create(datastore_path)?;
    let mut ts = TripleStore::create_persistent(metall_manager, crate::TRIPLESTORE_NAME)?;

    tracing::info!("Loading");

    unsafe {
        ts.load_ttl("/dev/stdin")
            .context("Could not load triples from ttl file")?;
    }
    tracing::info!("Loading complete");

    Ok(())
}
