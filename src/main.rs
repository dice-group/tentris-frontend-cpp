mod backup;
mod dump;
mod help;
mod load;
mod restore;
mod serve;

use clap::Parser;
use std::{
    net::{IpAddr, SocketAddr},
    path::PathBuf,
};


#[derive(Parser)]
#[command(version, about)]
struct Commandline {
    /// Path to use for persistent storage.
    #[clap(short = 's', long, default_value = std::env::current_dir().unwrap().join("tentris-data").into_os_string())]
    datastore_path: PathBuf,

    #[clap(subcommand)]
    sub: SubCommand,
}

#[derive(Parser)]
enum SubCommand {
    /// Load a dataset from a turtle stream
    #[clap(after_long_help = help::LOAD_EXTENDED_HELP)]
    Load,
    /// Run the server using a previously loaded dataset
    #[clap(after_long_help = help::SERVE_EXTENDED_HELP)]
    Serve(ServeOpts),
    /// Dump the database contents as n-triples
    #[clap(after_long_help = help::DUMP_EXTENDED_HELP)]
    Dump,
    /// Generate backup stream of database
    #[clap(after_long_help = help::BACKUP_EXTENDED_HELP)]
    Backup,
    /// Create database from backup stream
    #[clap(after_long_help = help::RESTORE_EXTENDED_HELP)]
    Restore,
}

#[derive(Parser)]
pub struct ServeOpts {
    /// The number of blocking threads used by the runtime to evaluate queries
    #[clap(short = 'j', long, default_value_t = num_cpus::get())]
    query_eval_threads: usize,

    /// The buffer size in bytes for serializing results
    #[clap(short = 'm', long, default_value_t = 4096 * 16)]
    query_eval_serialization_mem: usize,

    /// The number of io threads used by the runtime to accept queries/write out results
    #[clap(short = 'i', long, default_value_t = num_cpus::get())]
    io_threads: usize,

    /// Timeout in milliseconds for answering requests [default: no timeout]
    #[clap(long)]
    request_timeout_ms: Option<u64>,

    /// The address the server should bind to.
    /// Will listen on <BIND_ADDRESS>/stream and <BIND_ADDRESS>/sparql for a SPARQL query that is sent
    /// either using a GET request with the query url-encoded in the query= parameter (Content-Type: application/x-www-form-urlencoded)
    /// or a POST request with the query provided in the body (Content-Type: application/sparql-query)
    #[clap(default_value_t = SocketAddr::new(IpAddr::from([0, 0, 0, 0]), 9080))]
    bind_address: SocketAddr,
}

pub const TRIPLESTORE_NAME: &str = "tentris-triplestore";

fn main() {
    let opts: Commandline = Commandline::parse();

    tracing_subscriber::fmt().with_writer(std::io::stderr).init();

    match opts.sub {
        SubCommand::Load => load::load(&opts.datastore_path).unwrap(),
        SubCommand::Serve(serve_opts) => serve::serve(&opts.datastore_path, serve_opts).unwrap(),
        SubCommand::Dump => dump::dump(&opts.datastore_path).unwrap(),
        SubCommand::Backup => backup::backup(&opts.datastore_path).unwrap(),
        SubCommand::Restore => restore::restore(&opts.datastore_path).unwrap(),
    }
}
