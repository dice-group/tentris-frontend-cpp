use crate::serve::routes::{
    error::{QueryDeserializeError, UserFacingError},
    results_writer::{NTriplesResultsWriter, ResultsWriter, SparqlJsonSaxResultsWriter},
};
use axum::{extract::RawQuery, headers::ContentType};
use mime::Mime;
use serde::Deserialize;
use serde_json::{json, ser::CompactFormatter};
use std::io::Write;
use tentris::triplestore::{QueryType, SolutionGenerator};

#[derive(Deserialize)]
pub struct QueryParams {
    pub query: String,
}

#[derive(Deserialize)]
struct UpdateParams {
    update: String,
}

/// Extracts the update query from the request body
///
/// If content_type == application/sparql-update the query is found unencoded in the body
/// If content_type == application/www-form-url-encoded the query is found url-encoded in the body
/// otherwise an error is returned
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

/// Extracts the query from the request
///
/// If content_type == application/sparql-query the query is found unencoded in the body
/// If content_type == application/www-form-url-encoded the query is found url-encoded in the "query" parameter of the query part of the url
/// otherwise an error is returned
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

pub fn handle_ask_query(mut gen: SolutionGenerator) -> Result<Vec<u8>, tentris::triplestore::error::Error> {
    debug_assert!(gen.query_type() == QueryType::Ask);

    let ask_result = match gen.next_solution_mapping() {
        Ok(Some(_)) => true,
        Ok(None) => false,
        Err(e) => {
            return Err(e);
        },
    };

    Ok(serde_json::to_vec(&json!(
        {
            "head": {},
            "boolean": ask_result
        }
    ))
    .unwrap())
}

pub fn select_results_writer<W: Write + 'static>(gen: &SolutionGenerator, writer: W) -> Box<dyn ResultsWriter<W>> {
    match gen.query_type() {
        QueryType::Construct => Box::new(NTriplesResultsWriter::new(writer)),
        _ => Box::new(SparqlJsonSaxResultsWriter::new(
            writer,
            CompactFormatter,
            gen.projected_variables(),
        )),
    }
}
