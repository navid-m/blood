entry "./src/main.sx"
version "v0.1.0"
spectre_version "v0.0.18"

build release {
    flags "-release -dynamic"
    output "./blood"
}

build dev {
    flags "-dynamic"
    output "./blood-dev"
}
