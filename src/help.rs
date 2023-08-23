pub const LOAD_EXTENDED_HELP: &str = color_print::cstr!(
    r#"<bold><underline>Examples:</underline></bold>
  $ tentris -s datastore load << /home/user/dataset.nt
"#
);

pub const SERVE_EXTENDED_HELP: &str = color_print::cstr!(
    r#"<bold><underline>Examples:</underline></bold>
  $ tentris -s datastore serve
  $ tentris -s datastore serve 127.0.0.1:1234
"#
);

pub const DUMP_EXTENDED_HELP: &str = color_print::cstr!(
    r#"<bold><underline>Examples:</underline></bold>
  $ tentris -s datastore dump
  $ tentris -s datastore dump > /path/to/triples.nt
"#
);

pub const BACKUP_EXTENDED_HELP: &str = color_print::cstr!(
    r#"<bold><underline>Examples:</underline></bold>
  $ tentris -s datastoreA backup > backup.tar.zstd
  $ tentris -s datastoreA backup | ssh host "cat - > /path/to/backup.tar.zstd"
  $ tentris -s datastoreA backup | ssh host "tentris -s datastoreB restore"
"#
);

pub const RESTORE_EXTENDED_HELP: &str = color_print::cstr!(
    r#"<bold><underline>Examples:</underline></bold>
  $ tentris -s datastoreA restore << /path/to/backup.tar.zstd
  $ ssh host "tentris -s datastoreB" backup | tentris -s datastoreA restore
"#
);
