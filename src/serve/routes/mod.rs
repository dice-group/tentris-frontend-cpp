pub mod error;

mod results_writer;
mod utils;

use super::AppState;
use crate::serve::routes::utils::{
    extract_query, extract_update, handle_ask_query, select_results_writer, QueryParams,
};
use axum::{
    body::StreamBody,
    extract::{Query, RawQuery, State},
    headers::ContentType,
    http::{header, HeaderValue},
    response::{IntoResponse, Response},
    TypedHeader,
};
use error::UserFacingError;
use std::{io, mem};
use tentris::triplestore::QueryType;
use tokio::sync::{mpsc, oneshot};
use tokio_stream::wrappers::UnboundedReceiverStream;

/// GET /sparql
pub async fn sparql_route_get(
    State(state): State<AppState>,
    Query(query): Query<QueryParams>,
) -> Result<Response, UserFacingError> {
    route_impl(state, query.query).await
}

/// POST /sparql
pub async fn sparql_route_post(
    TypedHeader(content_type): TypedHeader<ContentType>,
    State(state): State<AppState>,
    raw_query: RawQuery,
    body: String,
) -> Result<Response, UserFacingError> {
    let query = extract_query(content_type, raw_query, body)?;
    route_impl(state, query).await
}

/// GET /stream
pub async fn sparql_streaming_route_get(
    State(state): State<AppState>,
    Query(query): Query<QueryParams>,
) -> Result<Response, UserFacingError> {
    streaming_route_impl(state, query.query).await
}

/// POST /stream
pub async fn sparql_streaming_route_post(
    TypedHeader(content_type): TypedHeader<ContentType>,
    State(state): State<AppState>,
    raw_query: RawQuery,
    body: String,
) -> Result<Response, UserFacingError> {
    let query = extract_query(content_type, raw_query, body)?;
    streaming_route_impl(state, query).await
}

/// POST /update
pub async fn sparql_update_route(
    TypedHeader(content_type): TypedHeader<ContentType>,
    State(state): State<AppState>,
    body: String,
) -> Result<(), UserFacingError> {
    let query = extract_update(content_type, body)?;
    update_route_impl(state, query).await
}

async fn route_impl(state: AppState, query: String) -> Result<Response, UserFacingError> {
    tracing::debug!("evaluating query={query:.80}");

    let (tx, rx) = oneshot::channel();

    rayon::spawn(move || {
        let init_sz = state.serialization_mem;

        let mut gen = match state.triplestore.eval_sparql_query(&query, state.request_timeout) {
            Ok(gen) => gen,
            Err(e) => {
                let _ = tx.send(Err(e));
                return;
            },
        };

        if gen.query_type() == QueryType::Ask {
            let _ = tx.send(handle_ask_query(gen));
            return;
        }

        {
            let mut writer = select_results_writer(&gen, Vec::with_capacity(init_sz));

            writer.begin().unwrap();

            loop {
                match gen.next_solution_mapping() {
                    Ok(Some(solution)) => {
                        writer.write_solution(solution).unwrap();
                    },
                    Ok(None) => break,
                    Err(e) => {
                        let _ = tx.send(Err(e));
                        return;
                    },
                }
            }

            writer.finish().unwrap();
            let _ = tx.send(Ok(mem::take(writer.as_inner_mut())));
        }
    });

    match rx.await {
        Ok(Ok(response)) => {
            let response_headers = [(
                header::CONTENT_TYPE,
                HeaderValue::from_static("application/sparql-results+json"),
            )];

            Ok((response_headers, response).into_response())
        },
        Ok(Err(e)) => Err(UserFacingError::EvalError(e)),
        Err(_) => Err(UserFacingError::Internal(io::Error::new(
            io::ErrorKind::Other,
            "unknown internal server error",
        ))),
    }
}

async fn streaming_route_impl(state: AppState, query: String) -> Result<Response, UserFacingError> {
    tracing::debug!("evaluating query={query:.80}");

    let (tx, mut rx) = mpsc::unbounded_channel();

    rayon::spawn(move || {
        let chunk_sz = state.serialization_mem;

        let mut gen = match state.triplestore.eval_sparql_query(&query, state.request_timeout) {
            Ok(gen) => {
                let Ok(_) = tx.send(Ok(vec![])) else {
                    return;
                };
                gen
            },
            Err(e) => {
                let _ = tx.send(Err(e));
                return;
            },
        };

        if gen.query_type() == QueryType::Ask {
            let _ = tx.send(handle_ask_query(gen));
            return;
        }

        {
            let mut writer = select_results_writer(&gen, Vec::with_capacity(chunk_sz));

            writer.begin().unwrap();

            loop {
                let res = match gen.next_solution_mapping() {
                    Ok(Some(solution)) => {
                        writer.write_solution(solution).unwrap();

                        let buf = writer.as_inner_mut();
                        if buf.len() >= chunk_sz {
                            tx.send(Ok(mem::replace(buf, Vec::with_capacity(chunk_sz))))
                        } else {
                            Ok(())
                        }
                    },
                    Ok(None) => break,
                    Err(e) => tx.send(Err(e)),
                };

                let Ok(_) = res else {
                    return;
                };
            }

            writer.finish().unwrap();
            let _ = tx.send(Ok(mem::take(writer.as_inner_mut())));
        }
    });

    match rx.recv().await {
        Some(Ok(_)) => {
            let response_stream = StreamBody::new(UnboundedReceiverStream::new(rx));
            let response_headers = [(
                header::CONTENT_TYPE,
                HeaderValue::from_static("application/sparql-results+json"),
            )];

            Ok((response_headers, response_stream).into_response())
        },
        Some(Err(e)) => Err(UserFacingError::EvalError(e)),
        None => Err(UserFacingError::Internal(io::Error::new(
            io::ErrorKind::Other,
            "unknown internal server error",
        ))),
    }
}

async fn update_route_impl(state: AppState, query: String) -> Result<(), UserFacingError> {
    tracing::debug!("evaluating update={query:.80}");

    let (tx, rx) = oneshot::channel();

    rayon::spawn(move || {
        let _ = tx.send(state.triplestore.eval_sparql_update(&query, state.request_timeout));
    });

    match rx.await {
        Ok(Ok(())) => Ok(()),
        Ok(Err(e)) => Err(UserFacingError::EvalError(e)),
        Err(_) => Err(UserFacingError::Internal(io::Error::new(
            io::ErrorKind::Other,
            "unknown internal server error",
        ))),
    }
}
