mod routes;

use axum::{error_handling::HandleErrorLayer, http::StatusCode, response::IntoResponse, routing::get, BoxError};
use clap::Parser;
use std::{
    net::{IpAddr, SocketAddr},
    path::PathBuf,
    sync::Arc,
    time::Duration,
};
use tentris::{metall::MetallManager, triplestore::TripleStore};
use tokio::signal::unix::SignalKind;

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
    /// Load the given file
    Load {
        /// Turtle file to load
        ttl_file: PathBuf,
    },
    /// Run the server using a previously loaded dataset
    Serve {
        /// The address the server should bind to.
        /// Will listen on <BIND_ADDRESS>/stream for a SPARQL request that is url-encoded in the query= parameter.
        #[clap(short = 'a', long, default_value_t = SocketAddr::new(IpAddr::from([0, 0, 0, 0]), 9080))]
        bind_address: SocketAddr,

        /// The number of blocking threads used by the runtime to evaluate queries
        #[clap(short = 'j', long, default_value_t = num_cpus::get())]
        query_eval_threads: usize,

        #[clap(short = 'm', long, default_value_t = 4096 * 16)]
        query_eval_serialization_mem: usize,

        /// The number of io threads used by the runtime to accept queries/write out results
        #[clap(short = 'i', long, default_value_t = num_cpus::get())]
        io_threads: usize,

        /// Timeout in milliseconds for answering requests [default: no timeout]
        #[clap(long)]
        request_timeout_ms: Option<u64>,
    },
}

const TRIPLESTORE_NAME: &str = "tentris-triplestore";

fn main() -> anyhow::Result<()> {
    let opts: Commandline = Commandline::parse();

    tracing_subscriber::fmt::init();

    match opts.sub {
        SubCommand::Load { ttl_file } => {
            let metall_manager = MetallManager::create(&opts.datastore_path)?;
            let mut ts = TripleStore::create_persistent(metall_manager, TRIPLESTORE_NAME)?;

            tracing::info!("Loading {}", ttl_file.display());
            unsafe {
                ts.load_ttl(&ttl_file)?;
            }
            tracing::info!("Loading complete");

            Ok(())
        },
        SubCommand::Serve {
            bind_address,
            query_eval_threads,
            query_eval_serialization_mem,
            io_threads,
            request_timeout_ms,
        } => {
            let runtime = tokio::runtime::Builder::new_multi_thread()
                .enable_io()
                .worker_threads(io_threads)
                .thread_name("tentris-worker")
                .build()?;

            rayon::ThreadPoolBuilder::new()
                .num_threads(query_eval_threads)
                .build_global()
                .unwrap();

            let metall_manager = MetallManager::open(&opts.datastore_path)?;
            let triplestore = Arc::new(TripleStore::open_persistent(metall_manager, TRIPLESTORE_NAME)?);

            let state = routes::AppState {
                triplestore,
                request_timeout: request_timeout_ms.map(Duration::from_millis),
                serialization_mem: query_eval_serialization_mem,
            };

            runtime.block_on(async move {
                let app = axum::Router::new()
                    .route(
                        "/stream",
                        get(routes::sparql_streaming_route_get).post(routes::sparql_streaming_route_post),
                    )
                    .route("/sparql", get(routes::sparql_route_get))
                    .with_state(state.clone())
                    .layer(
                        tower::ServiceBuilder::new()
                            .layer(HandleErrorLayer::new(handle_load_shed_error))
                            .load_shed()
                            .concurrency_limit(query_eval_threads),
                    );

                tracing::info!("Starting to listen on {0}/sparql and {0}/stream", bind_address);

                let server = axum::Server::bind(&bind_address).serve(app.into_make_service());

                let mut sigterm = tokio::signal::unix::signal(SignalKind::terminate()).unwrap();
                let mut sigint = tokio::signal::unix::signal(SignalKind::interrupt()).unwrap();

                tokio::select! {
                    _ = sigterm.recv() => Ok(()),
                    _ = sigint.recv() => Ok(()),
                    res = server => res,
                }
            })?;

            Ok(())
        },
    }
}

async fn handle_load_shed_error(_err: BoxError) -> impl IntoResponse {
    tracing::warn!("Shedding Load");
    StatusCode::TOO_MANY_REQUESTS
}
