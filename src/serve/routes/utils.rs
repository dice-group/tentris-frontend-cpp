use crate::serve::routes::error::{QueryDeserializeError, UserFacingError};
use axum::{extract::RawQuery, headers::ContentType};
use mime::Mime;
use serde::Deserialize;

#[derive(Deserialize)]
pub struct QueryParams {
    pub query: String,
}

#[derive(Deserialize)]
struct UpdateParams {
    update: String,
}

pub fn extract_update(content_type: ContentType, body: String) -> Result<String, UserFacingError> {
    let url_encoded_content_type = ContentType::form_url_encoded();
    let body_content_type = ContentType::from("application/sparql-update".parse::<Mime>().unwrap());

    let query = if content_type == body_content_type {
        body
    } else if content_type == url_encoded_content_type {
        let body = serde_urlencoded::from_str::<UpdateParams>(&body).map_err(|_| {
            UserFacingError::QueryDeserializeError(QueryDeserializeError::QueryNotPresent {
                parameter_name: "update".to_string(),
            })
        })?;

        body.update
    } else {
        return Err(UserFacingError::InvalidContentType {
            expected: vec![body_content_type, url_encoded_content_type],
            got: content_type,
        });
    };

    if query.is_empty() {
        Err(UserFacingError::QueryDeserializeError(
            QueryDeserializeError::EmptyQuery,
        ))
    } else {
        Ok(query)
    }
}

pub fn extract_query(content_type: ContentType, raw_query: RawQuery, body: String) -> Result<String, UserFacingError> {
    let query_param_content_type = ContentType::form_url_encoded();
    let body_content_type = ContentType::from("application/sparql-query".parse::<Mime>().unwrap());

    let query = if content_type == body_content_type {
        body
    } else if content_type == query_param_content_type {
        raw_query.0.ok_or_else(|| {
            UserFacingError::QueryDeserializeError(QueryDeserializeError::QueryNotPresent {
                parameter_name: "query".to_string(),
            })
        })?
    } else {
        return Err(UserFacingError::InvalidContentType {
            expected: vec![body_content_type, query_param_content_type],
            got: content_type,
        });
    };

    if query.is_empty() {
        Err(UserFacingError::QueryDeserializeError(
            QueryDeserializeError::EmptyQuery,
        ))
    } else {
        Ok(query)
    }
}
