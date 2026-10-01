from pathlib import Path

Import("env")

version_file = Path(env.subst("$PROJECT_DIR")) / "build_version.txt"
parts = version_file.read_text(encoding="ascii").strip().split(".")
if len(parts) != 3 or not all(part.isdigit() for part in parts):
    raise ValueError("build_version.txt must contain a semantic version such as 1.0.0")

major, minor, patch = (int(part) for part in parts)
version = f"{major}.{minor}.{patch + 1}"
version_file.write_text(version + "\n", encoding="ascii")
print(f"Auto-incremented firmware version to {version}")
