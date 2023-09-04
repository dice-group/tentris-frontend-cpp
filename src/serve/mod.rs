mod auth;
mod routes;

use super::ServeOpts;
use anyhow::Context;
use auth::SetupAuth;
use axum::{
    error_handling::HandleErrorLayer,
    http::StatusCode,
    response::IntoResponse,
    routing::{get, post},
    BoxError,
};
use std::{fs::File, path::Path, sync::Arc, time::Duration};
use tentris::{metall::MetallManager, triplestore::TripleStore};
use tokio::signal::unix::SignalKind;
use tower_http::trace::TraceLayer;

#[derive(Clone)]
pub struct AppState {
    pub triplestore: Arc<TripleStore>,
    pub request_timeout: Option<Duration>,
    pub serialization_mem: usize,
}

pub fn serve(
    datastore_path: &Path,
    ServeOpts {
        bind_address,
        query_eval_threads,
        query_eval_serialization_mem,
        io_threads,
        credentials,
        request_timeout_ms,
    }: ServeOpts,
) -> anyhow::Result<()> {
    let credentials: Option<auth::Credentials> = match credentials {
        Some(credentials) => {
            let f = File::open(credentials).context("Failed to open credentials file")?;
            Some(serde_json::from_reader(f).context("Failed to parse credentials")?)
        },
        None => None,
    };

    let runtime = tokio::runtime::Builder::new_multi_thread()
        .enable_io()
        .worker_threads(io_threads)
        .thread_name("tentris-worker")
        .build()?;

    rayon::ThreadPoolBuilder::new()
        .num_threads(query_eval_threads)
        .build_global()
        .context("Could not create worker thread pool")?;

    if !MetallManager::is_consistent(datastore_path) {
        anyhow::bail!("Datastore is inconsistent");
    }

    let metall_manager = MetallManager::open(datastore_path).context("Could not open datastore")?;

    let triplestore = Arc::new(
        TripleStore::open_persistent(metall_manager, crate::TRIPLESTORE_NAME)
            .context("Could not open persisted triplestore")?,
    );

    let state = AppState {
        triplestore,
        request_timeout: request_timeout_ms.map(Duration::from_millis),
        serialization_mem: query_eval_serialization_mem,
    };

    runtime.block_on(async move {
        let app = axum::Router::new()
            .route(
                "/stream",
                get(routes::sparql_streaming_route_get)
                    .post(routes::sparql_streaming_route_post)
                    .setup_read_only_auth(credentials.as_ref()),
            )
            .route(
                "/sparql",
                get(routes::sparql_route_get)
                    .post(routes::sparql_route_post)
                    .setup_read_only_auth(credentials.as_ref()),
            )
            .route(
                "/update",
                post(routes::sparql_update_route).setup_read_write_auth(credentials.as_ref()),
            )
            .route(
                "/update_data",
                post(routes::sparql_update_data_route).setup_read_write_auth(credentials.as_ref()),
            )
            .with_state(state.clone())
            .layer(TraceLayer::new_for_http())
            .layer(
                tower::ServiceBuilder::new()
                    .layer(HandleErrorLayer::new(handle_load_shed_error))
                    .load_shed()
                    .concurrency_limit(query_eval_threads),
            );

        tracing::info!("Starting to listen on {bind_address}/{{sparql,stream,update,update_data}}");

        let server = axum::Server::bind(&bind_address).serve(app.into_make_service());

        let mut sigterm = tokio::signal::unix::signal(SignalKind::terminate())
            .context("Could not create signal handler for SIGTERM")
            .unwrap();

        let mut sigint = tokio::signal::unix::signal(SignalKind::interrupt())
            .context("Could not create signal handler for SIGINT")
            .unwrap();

        tokio::select! {
            _ = sigterm.recv() => Ok(()),
            _ = sigint.recv() => Ok(()),
            res = server => res,
        }
    })?;

    Ok(())
}

async fn handle_load_shed_error(_err: BoxError) -> impl IntoResponse {
    tracing::warn!("Shedding Load");
    StatusCode::TOO_MANY_REQUESTS
}
