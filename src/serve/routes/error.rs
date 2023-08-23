use axum::{
    headers::ContentType,
    http::StatusCode,
    response::{IntoResponse, Response},
};
use serde_json::json;
use std::io;
use tentris::triplestore::error::Error;
use thiserror::Error;

#[derive(Debug, Error)]
pub enum UserFacingError {
    #[error("An internal error occurred: {0}")]
    Internal(io::Error),

    #[error("Bad request: eval error {0}")]
    EvalError(#[from] Error),

    #[error("Received invalid content type, expected any of {expected:?} but got {got} instead")]
    InvalidContentType {
        expected: Vec<ContentType>,
        got: ContentType,
    },

    #[error("Unauthorized")]
    Unauthorized,

    #[error("Empty query provided")]
    EmptyQuery { provided_content_type: ContentType },
}

impl IntoResponse for UserFacingError {
    fn into_response(self) -> Response {
        let (code, body) = match self {
            UserFacingError::Internal(e) => {
                let body = json! {
                    {
                        "Internal": {
                            "error_kind": e.kind().to_string(),
                            "error_message": format!("{e}"),
                        }
                    }
                };

                (StatusCode::INTERNAL_SERVER_ERROR, body)
            },
            UserFacingError::EvalError(e) => {
                let body = json! {
                    {
                        "EvalError": e,
                    }
                };

                (StatusCode::BAD_REQUEST, body)
            },
            UserFacingError::InvalidContentType { expected, got } => {
                let body = json! {
                    {
                        "InvalidContentType": {
                            "expected": format!("{expected:?}"),
                            "got": format!("{got}"),
                            "error_message": format!("Invalid content type, expected any of {expected:?} but got {got} instead"),
                        }
                    }
                };

                (StatusCode::BAD_REQUEST, body)
            },
            UserFacingError::Unauthorized => {
                let body = json! {
                    {
                        "Unauthorized": {}
                    }
                };

                (StatusCode::UNAUTHORIZED, body)
            },
            UserFacingError::EmptyQuery { provided_content_type } => {
                let body = json! {
                    {
                        "EmptyQuery": {
                            "provided_content_type": provided_content_type.to_string(),
                        }
                    }
                };

                (StatusCode::BAD_REQUEST, body)
            },
        };

        (code, serde_json::to_string(&body).unwrap()).into_response()
    }
}
