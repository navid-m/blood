entry "./src/main.sx"
version "v0.1.0"
spectre_version "v0.0.18"

build release {
    flags "-release"
    output "./blood"
}

build dev {
    flags ""
    output "./blood-dev"
}
