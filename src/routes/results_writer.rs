use serde_json::ser::Formatter;
use std::{io, io::Write, mem};
use tentris::rdf4cpp::{NodeType, SolutionMapping, Variable};

pub struct SparqlJsonSaxResultsWriter<W, F> {
    proj_vars: Vec<String>,
    writer: W,
    fmt: F,
    first_binding: bool,
}

fn begin_field<W: Write, F: Formatter>(fmt: &mut F, mut writer: W, field_name: &str, first: bool) -> io::Result<()> {
    fmt.begin_object_key(&mut writer, first)?;
    serde_json::to_writer(&mut writer, field_name)?;
    fmt.end_object_key(&mut writer)?;
    fmt.begin_object_value(&mut writer)?;
    Ok(())
}

fn end_field<W: Write, F: Formatter>(fmt: &mut F, mut writer: W) -> io::Result<()> {
    fmt.end_object_value(&mut writer)?;
    Ok(())
}

fn string_field<W: Write, F: Formatter>(
    fmt: &mut F,
    mut writer: W,
    field_name: &str,
    field_value: &str,
    first: bool,
) -> io::Result<()> {
    begin_field(fmt, &mut writer, field_name, first)?;
    serde_json::to_writer(&mut writer, field_value)?;
    end_field(fmt, &mut writer)?;
    Ok(())
}

impl<W: Write, F: Formatter> SparqlJsonSaxResultsWriter<W, F> {
    pub unsafe fn new(writer: W, fmt: F, proj_vars: &[Variable]) -> Self {
        Self {
            proj_vars: proj_vars.iter().map(|&var| (*var.name()).to_owned()).collect(),
            writer,
            fmt,
            first_binding: true,
        }
    }

    pub fn writer_mut(&mut self) -> &mut W {
        &mut self.writer
    }

    pub fn begin(&mut self) -> io::Result<()> {
        self.fmt.begin_object(&mut self.writer)?;
        {
            begin_field(&mut self.fmt, &mut self.writer, "head", true)?;
            self.fmt.begin_object(&mut self.writer)?;
            {
                begin_field(&mut self.fmt, &mut self.writer, "vars", true)?;
                self.fmt.begin_array(&mut self.writer)?;
                for (ix, var) in self.proj_vars.iter().enumerate() {
                    self.fmt.begin_array_value(&mut self.writer, ix == 0)?;
                    serde_json::to_writer(&mut self.writer, var)?;
                    self.fmt.end_array_value(&mut self.writer)?;
                }
                self.fmt.end_array(&mut self.writer)?;
                end_field(&mut self.fmt, &mut self.writer)?;
            }
            self.fmt.end_object(&mut self.writer)?;
            end_field(&mut self.fmt, &mut self.writer)?;
        }

        {
            begin_field(&mut self.fmt, &mut self.writer, "results", false)?;
            self.fmt.begin_object(&mut self.writer)?;
            {
                begin_field(&mut self.fmt, &mut self.writer, "bindings", true)?;
                self.fmt.begin_array(&mut self.writer)?;
            }
        }

        Ok(())
    }

    pub fn finish(&mut self) -> io::Result<()> {
        self.fmt.end_array(&mut self.writer)?;
        end_field(&mut self.fmt, &mut self.writer)?;
        self.fmt.end_object(&mut self.writer)?;
        end_field(&mut self.fmt, &mut self.writer)?;
        self.fmt.end_object(&mut self.writer)?;
        Ok(())
    }

    pub fn write_solution(&mut self, solution: SolutionMapping) -> io::Result<()> {
        debug_assert!(self.proj_vars.len() == solution.mapping.len());

        self.fmt
            .begin_array_value(&mut self.writer, mem::take(&mut self.first_binding))?;
        self.fmt.begin_object(&mut self.writer)?;

        for (ix, (var, term)) in self.proj_vars.iter().zip(solution.mapping).enumerate() {
            begin_field(&mut self.fmt, &mut self.writer, var, ix == 0)?;

            if term.is_null() {
                self.fmt.begin_string(&mut self.writer)?;
                self.fmt.write_string_fragment(&mut self.writer, "null")?;
                self.fmt.end_string(&mut self.writer)?;
            } else {
                self.fmt.begin_object(&mut self.writer)?;

                match term.node_type() {
                    NodeType::Iri => {
                        let value = unsafe { term.to_iri().identifier() };
                        string_field(&mut self.fmt, &mut self.writer, "type", "iri", true)?;
                        string_field(&mut self.fmt, &mut self.writer, "value", value, false)?;
                    },
                    NodeType::BNode => {
                        let value = unsafe { term.to_bnode().identifier() };
                        string_field(&mut self.fmt, &mut self.writer, "type", "bnode", true)?;
                        string_field(&mut self.fmt, &mut self.writer, "value", value, false)?;
                    },
                    NodeType::Literal => {
                        let lit = term.to_literal();
                        let lex = unsafe { lit.lexical_form() };

                        string_field(&mut self.fmt, &mut self.writer, "type", "literal", true)?;
                        string_field(
                            &mut self.fmt,
                            &mut self.writer,
                            "value",
                            &lex,
                            false,
                        )?;

                        let lang_tag = unsafe { lit.language_tag() };
                        if !lang_tag.is_empty() {
                            string_field(&mut self.fmt, &mut self.writer, "xml:lang", lang_tag, false)?;
                        } else {
                            string_field(
                                &mut self.fmt,
                                &mut self.writer,
                                "datatype",
                                unsafe { lit.datatype() },
                                false,
                            )?;
                        }
                    },
                    NodeType::Variable => unreachable!(),
                }

                self.fmt.end_object(&mut self.writer)?;
            }

            end_field(&mut self.fmt, &mut self.writer)?;
        }

        self.fmt.end_object(&mut self.writer)?;
        self.fmt.end_array_value(&mut self.writer)?;
        Ok(())
    }
}
