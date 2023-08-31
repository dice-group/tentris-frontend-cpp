use axum::{
    headers::ContentType,
    http::StatusCode,
    response::{IntoResponse, Response},
};
use std::io;
use tentris::triplestore::error::Error;
use thiserror::Error;

#[derive(Debug, Error)]
pub enum UserFacingError {
    #[error("An internal error occurred: {0}")]
    Internal(io::Error),

    #[error("Eval error {0}")]
    EvalError(#[from] Error),

    #[error("Received invalid content-type, expected any of {expected:?} but got {got} instead")]
    InvalidContentType {
        expected: Vec<ContentType>,
        got: ContentType,
    },

    #[error("Failed to deserialize query: {0}")]
    QueryDeserializeError(QueryDeserializeError),
}

#[derive(Debug, Error)]
pub enum QueryDeserializeError {
    #[error("Empty query provided")]
    EmptyQuery,

    #[error("Query not present in parameter {parameter_name}")]
    QueryNotPresent { parameter_name: String },
}

impl IntoResponse for UserFacingError {
    fn into_response(self) -> Response {
        let code = match &self {
            UserFacingError::Internal(_) => StatusCode::INTERNAL_SERVER_ERROR,
            UserFacingError::EvalError(_)
            | UserFacingError::InvalidContentType { .. }
            | UserFacingError::QueryDeserializeError(_) => StatusCode::BAD_REQUEST,
        };

        (code, format!("{self}")).into_response()
    }
}
