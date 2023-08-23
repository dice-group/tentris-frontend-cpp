pub mod error;
mod results_writer;

use axum::{
    body::StreamBody,
    extract::{Query, State},
    headers::ContentType,
    http::{header, HeaderValue},
    response::{IntoResponse, Response},
    TypedHeader,
};
use error::UserFacingError;
use mime::Mime;
use results_writer::SparqlJsonSaxResultsWriter;
use serde::Deserialize;
use serde_json::ser::CompactFormatter;
use std::{io, mem, sync::Arc, time::Duration};
use tentris::triplestore::TripleStore;
use tokio::sync::{mpsc, oneshot};
use tokio_stream::wrappers::UnboundedReceiverStream;

#[derive(Clone)]
pub struct AppState {
    pub triplestore: Arc<TripleStore>,
    pub request_timeout: Option<Duration>,
    pub serialization_mem: usize,
}

#[derive(Deserialize)]
pub struct QueryParams {
    #[serde(default)]
    query: String,
}

async fn route_impl(state: AppState, query: String) -> Result<Response, UserFacingError> {
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

        {
            let mut writer = unsafe {
                SparqlJsonSaxResultsWriter::new(
                    Vec::with_capacity(init_sz),
                    CompactFormatter,
                    gen.projected_variables(),
                )
            };

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
            let _ = tx.send(Ok(mem::take(writer.writer_mut())));
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

        {
            let mut writer = unsafe {
                SparqlJsonSaxResultsWriter::new(
                    Vec::with_capacity(chunk_sz),
                    CompactFormatter,
                    gen.projected_variables(),
                )
            };

            writer.begin().unwrap();

            loop {
                let res = match gen.next_solution_mapping() {
                    Ok(Some(solution)) => {
                        writer.write_solution(solution).unwrap();

                        let buf = writer.writer_mut();
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
            let _ = tx.send(Ok(mem::take(writer.writer_mut())));
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

pub async fn sparql_route_get(
    State(state): State<AppState>,
    Query(params): Query<QueryParams>,
) -> Result<Response, UserFacingError> {
    route_impl(state, params.query).await
}

pub async fn sparql_streaming_route_get(
    State(state): State<AppState>,
    Query(params): Query<QueryParams>,
) -> Result<Response, UserFacingError> {
    streaming_route_impl(state, params.query).await
}

pub async fn sparql_streaming_route_post(
    TypedHeader(content_type): TypedHeader<ContentType>,
    State(state): State<AppState>,
    Query(params): Query<QueryParams>,
    body: String,
) -> Result<Response, UserFacingError> {
    let body_content_type = ContentType::from("application/sparql-query".parse::<Mime>().unwrap());
    let query_param_content_type = ContentType::form_url_encoded();

    if content_type == body_content_type {
        streaming_route_impl(state, body).await
    } else if content_type == query_param_content_type {
        streaming_route_impl(state, params.query).await
    } else {
        Err(UserFacingError::InvalidContentType {
            expected: vec![body_content_type, query_param_content_type],
            got: content_type,
        })
    }
}
