# 🌺💜 Violet: Extended C++ standard library
# Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

_cc_kinds = [
    "cc_library",
    "cc_test",
]

ExportInfo = provider(
    doc = "Carries information about the export manifest file for a single target",
    fields = {
        "manifests": "depset of manifest JSON files",
    },
)

def _file_path(f):
    """Returns the short_path for a File, stripping any leading ../."""
    p = f.short_path
    if p.startswith("../"):
        return p

    return p

def _extract_files(attr_name, ctx):
    """Extract file paths from a rule attribute that contains file targets."""
    attr = getattr(ctx.rule.attr, attr_name, None)
    if attr == None:
        return []

    result = []
    for target in attr:
        for f in target.files.to_list():
            result.append(_file_path(f))

    return result

def _extract_deps(ctx):
    deps = []
    for dep in getattr(ctx.rule.attr, "deps", []):
        deps.append(str(dep.label))

    return deps

def _extract_string_list(ctx, attr_name):
    """Extract a plain string list attribute."""
    return list(getattr(ctx.rule.attr, attr_name, []))

def _try_extract_selects(_):
    """
    Attempt to extract select() information.
    NOTE: Bazel aspects see the *resolved* configuration, not raw selects.
    This is a best-effort extraction from rule attributes; full select()
    introspection requires `bazel query --output=build` or buildozer.
    Returns an empty dict as a placeholder — the generator script should
    merge in select data from a separate `bazel query` pass.
    """
    return {}

def _export_aspect_impl(target, ctx):
    kind = ctx.rule.kind
    if kind not in _cc_kinds:
        manifests = []
        for dep in getattr(ctx.rule.attr, "deps", []):
            if ExportInfo in dep:
                manifests.append(dep[ExportInfo].manifests)

        return [ExportInfo(manifests = depset(transitive = manifests))]

    deps = _extract_deps(ctx)
    manifest = {
        "copts": _extract_string_list(ctx, "copts"),
        "defines": _extract_string_list(ctx, "defines"),
        "deps": deps,
        "hdrs": _extract_files("hdrs", ctx),
        "kind": kind,
        "label": {
            "name": target.label.name,
            "package": target.label.package,
            "string": str(target.label),
        },
        "linkopts": _extract_string_list(ctx, "linkopts"),
        "local_defines": _extract_string_list(ctx, "local_defines"),
        "selects": _try_extract_selects(ctx),
        "srcs": _extract_files("srcs", ctx),
        "tags": _extract_string_list(ctx, "tags"),
        "visibility": [str(v) for v in getattr(ctx.rule.attr, "visibility", [])],
    }

    out = ctx.actions.declare_file("%s.export.json" % target.label.name)
    ctx.actions.write(out, content = json.encode(manifest))

    # Collect child manifests from deps
    child_manifests = []
    for dep in getattr(ctx.rule.attr, "deps", []):
        if ExportInfo in dep:
            child_manifests.append(dep[ExportInfo].manifests)

    return [
        ExportInfo(manifests = depset(direct = [out], transitive = child_manifests)),
        OutputGroupInfo(exported = depset(direct = [out], transitive = child_manifests)),
    ]

export_aspect = aspect(
    implementation = _export_aspect_impl,
    attr_aspects = ["deps"],
    doc = "Extracts violet_cc_* target metadata into JSON IR for CMake/Meson generation",
)
