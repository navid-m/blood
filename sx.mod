entry "./src/main.sx"
version "v0.0.1"
spectre_version "v0.0.17"

build release {
    flags "-release"
    output "./blood"
}

build dev {
    output "./blood-dev"
}
