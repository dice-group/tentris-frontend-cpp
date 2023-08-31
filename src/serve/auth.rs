use axum::routing::MethodRouter;
use serde::Deserialize;
use tower_http::validate_request::ValidateRequestHeaderLayer;

#[derive(Deserialize)]
struct User {
    username: String,
    password: String,
}

#[derive(Deserialize)]
pub struct Credentials {
    read_only_user: User,
    read_write_user: User,
}

pub trait SetupAuth {
    fn setup_read_only_auth(self, creds: Option<&Credentials>) -> Self;
    fn setup_read_write_auth(self, creds: Option<&Credentials>) -> Self;
}

impl<S: Clone + 'static> SetupAuth for MethodRouter<S> {
    fn setup_read_only_auth(self, creds: Option<&Credentials>) -> Self {
        match creds {
            Some(creds) => self.layer(ValidateRequestHeaderLayer::basic(
                &creds.read_only_user.username,
                &creds.read_write_user.password,
            )),
            None => self,
        }
    }

    fn setup_read_write_auth(self, creds: Option<&Credentials>) -> Self {
        match creds {
            Some(creds) => self.layer(ValidateRequestHeaderLayer::basic(
                &creds.read_write_user.username,
                &creds.read_write_user.password,
            )),
            None => self,
        }
    }
}
