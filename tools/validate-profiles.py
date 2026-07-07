#!/usr/bin/env python3
"""Validate profiles/examples/*.json against profiles/schema.json."""

from __future__ import annotations

import json
import sys
from pathlib import Path

try:
    import jsonschema
except ImportError:
    print("jsonschema is required: pip install jsonschema", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "profiles" / "schema.json"
EXAMPLES_DIR = ROOT / "profiles" / "examples"


METADATA_KEYS = {"$schema", "title", "description"}


def profile_payload(data: dict) -> dict:
    return {key: value for key, value in data.items() if key not in METADATA_KEYS}


def main() -> int:
    schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
    validator = jsonschema.Draft202012Validator(schema)

    if not EXAMPLES_DIR.is_dir():
        print(f"No examples directory: {EXAMPLES_DIR}", file=sys.stderr)
        return 1

    errors = 0
    for path in sorted(EXAMPLES_DIR.glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        payload = profile_payload(data)
        issues = sorted(validator.iter_errors(payload), key=lambda e: e.path)
        if issues:
            errors += 1
            print(f"FAIL {path.relative_to(ROOT)}")
            for issue in issues:
                loc = ".".join(str(p) for p in issue.path) or "(root)"
                print(f"  {loc}: {issue.message}")
        else:
            print(f"OK   {path.relative_to(ROOT)}")

    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
