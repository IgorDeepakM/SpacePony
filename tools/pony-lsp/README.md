# pony-lsp - the Pony Language Server

This language server is communicating with LSP-clients (editors) via stdout/stdin.

## Feature Support

| **Feature** | **Description** |
| ------- | ----------- |
| **[Diagnostics][spec-diagnostics]** (push, pull, refresh and change notifications) | Ponyc errors and related information is reported as LSP diagnostics. |
| **[Hover][spec-hover]** | Additional information is shown for: entities, methods, fields, local variables and references. |
| **[Signature Help][spec-signature-help]** | Parameter hints are shown when the cursor is inside a call expression, with the active parameter highlighted. Includes the method's docstring when present. Requires the file to be saved — signature help is not available while the file has unsaved edits. |
| **[Go To Definition][spec-definition]** | For most language constructs, you can go from a reference to its definition. |
| **[Go To Declaration][spec-declaration]** | Navigate from a reference to the declaration site of a symbol. |
| **[Go To Type Definition][spec-type-definition]** | Navigate to the type definition of the symbol under the cursor. |
| **[Document Symbols][spec-document-symbols]** | pony-lsp provides a list of available symbols for each opened document. |
| **[Workspace Symbols][spec-workspace-symbols]** | Fuzzy search over all symbols across the entire workspace. |
| **[Document Highlight][spec-document-highlight]** | All occurrences of the symbol under the cursor are highlighted simultaneously across the document. |
| **[Inlay Hints][spec-inlay-hints]** | Three kinds of hints are shown inline. For `let`, `var`, and field declarations with no type annotation, the full inferred type appears after the variable name. For annotated types where the capability is omitted — in variable and field declarations, function parameter types, generic type arguments, and union and tuple members — the missing capability keyword appears after the type name. For `fun` declarations, the implicit receiver capability appears before the function name and the implicit return type appears after the closing parenthesis. |
| **[Find References][spec-references]** | All references to the symbol under the cursor are returned, with optional inclusion of the declaration site. |
| **[Rename][spec-rename]** | Rename a symbol and all its references across the workspace. |
| **[Folding Range][spec-folding-range]** | Code folding ranges are provided for blocks, methods, classes, and other structured constructs. |
| **[Selection Range][spec-selection-range]** | Smart expand/shrink selection based on the AST structure of the document. |

[spec-diagnostics]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#pull-diagnostics
[spec-hover]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#hover-request-leftwards_arrow_with_hook
[spec-signature-help]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_signatureHelp
[spec-definition]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#goto-definition-request-leftwards_arrow_with_hook
[spec-declaration]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#goto-declaration-request-leftwards_arrow_with_hook
[spec-type-definition]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#goto-type-definition-request-leftwards_arrow_with_hook
[spec-document-symbols]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#document-symbols-request-leftwards_arrow_with_hook
[spec-workspace-symbols]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspace-symbols-request-leftwards_arrow_with_hook
[spec-document-highlight]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#document-highlights-request-leftwards_arrow_with_hook
[spec-inlay-hints]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlay-hint-request-leftwards_arrow_with_hook
[spec-references]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#find-references-request-leftwards_arrow_with_hook
[spec-rename]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#rename-request-leftwards_arrow_with_hook
[spec-folding-range]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#folding-range-request-leftwards_arrow_with_hook
[spec-selection-range]: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#selection-range-request-leftwards_arrow_with_hook

We are constantly working on adding new features, but also welcome all contributions and offer help and guidance for implementing any feature.

## Settings

`pony-lsp` supports settings via `workspace/configuration` request and `workspace/didChangeConfiguration`.
It expects the following optional settings:

| **Name** | **Type** | **Example** | **Description** |
| ---- | ---- | ------- | ----------- |
| **defines** | `Array[String]` | `["FOO", "BAR"]` | Defines active during compilation. These are usually set when using `ponyc` using `-D` |
| **ponypath** | `Array[String]` | `["/path/to/pony/package", "/another/path"]` | An array of paths which are added to the package search paths of `pony-lsp` |

Example settings in JSON:

```json
{
  "defines": ["FOO", "BAR"],
  "ponypath": ["/path/to/pony/package", "/another/path"]
}
```

## Getting pony-lsp

`pony-lsp` is distributed via `ponyup` alongside ponyc, so installing a recent `ponyc` will also give you `pony-lsp`.

Checkout the [Ponyc installation instructions](/INSTALL.md) on how to install `ponyc` (and `pony-lsp` at the same time).

## Building from source

First checkout [the ponyc build documentation](/BUILD.md) and make sure you have
everything necessary available to build `ponyc`. Then building `pony-lsp` should be as simple as:

```bash
make pony-lsp
```

## Reporting Issues

The best support and feedback you can give us, is when the pony-lsp is not working as expected.
Please provide a minimal Pony program to reproduce this issue, document the `pony-lsp` version you are using and the lsp-client/editor you are using.
