entry "./src/main.sx"
version "v0.1.3"
spectre_version "v0.0.18"

build sl {
    action "mkdir -p ./generated && spectre ./src/main.sx -src-target:linux -release -b -emit-alt -d:GIT_HASH=$(git rev-parse --short HEAD) > ./generated/blood_linux.c"
}

build sm {
    action "mkdir -p ./generated && spectre ./src/main.sx -src-target:darwin -release -b -emit-alt -d:GIT_HASH=$(git rev-parse --short HEAD) > ./generated/blood_darwin.c"
}


build release {
    flags "-release"
    output "./blood"
    env {
        GIT_HASH=$((git rev-parse --short HEAD))
    }
    darwin {
        flags "-release -runtime:hosted"
    }
}

build dev {
    flags ""
    output "./blood-dev"
    env {
        GIT_HASH=$((git rev-parse --short HEAD))
    }
}
