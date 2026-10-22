#!/bin/sh
# Builds and smoke-tests the Linux prebuild inside a container. Expects the
# repository mounted at /work. Used by .github/workflows/mdns-browse-build.yml:
#
#   glibc: quay.io/pypa/manylinux_2_28_{x86_64,aarch64} (AlmaLinux 8, glibc 2.28)
#          with the runner's Node.js mounted read-only at /opt/hostnode
#   musl:  node:24-alpine
#
# Environment: HOST_UID / HOST_GID (optional) are used to hand ownership of
# the workspace back to the runner user at the end.
set -eu

cd /work

if [ -f /etc/alpine-release ]; then
  libc=musl
  apk add --no-cache python3 make g++ linux-headers binutils avahi-dev
else
  libc=glibc
  # avahi-devel lives in the PowerTools repo (called crb on newer releases).
  dnf install -y --enablerepo=powertools avahi-devel \
    || dnf install -y --enablerepo=crb avahi-devel \
    || dnf install -y avahi-devel
  # Node.js from the host runner (official binaries need glibc >= 2.28) and a
  # recent Python for node-gyp.
  export PATH="/opt/hostnode/bin:/opt/python/cp312-cp312/bin:$PATH"
fi

echo "== toolchain ($libc) =="
node --version
npm --version
python3 --version
g++ --version | head -n 1

npm ci --ignore-scripts
npm run prebuildify -w mdns-browse-build -- --tag-libc

cd mdns-browse-build
echo "== prebuilds =="
find prebuilds -type f -exec ls -l {} \;

for f in prebuilds/*/*.node; do
  echo "== dynamic dependencies of $f =="
  ldd "$f" || true

  if [ "$libc" = glibc ]; then
    # The addon must not need a newer glibc than the oldest Node.js release we
    # support (glibc 2.28).
    max=$(objdump -T "$f" | grep -o 'GLIBC_[0-9][0-9.]*' | sed 's/^GLIBC_//' \
      | sort -uV | tail -n 1)
    echo "highest required GLIBC symbol version: ${max:-none}"
    objdump -T "$f" | grep -o 'GLIBCXX_[0-9][0-9.]*' | sort -uV | tail -n 1 \
      | sed 's/^/highest required GLIBCXX symbol version: /' || true
    if [ -n "$max" ]; then
      newest=$(printf '%s\n2.28\n' "$max" | sort -V | tail -n 1)
      if [ "$newest" != "2.28" ]; then
        echo "error: $f requires GLIBC_$max, newer than the 2.28 baseline" >&2
        exit 1
      fi
    fi
  fi
done

# Load the fresh prebuild through the real package layout.
npm run link-local
npm run smoke

if [ -n "${HOST_UID:-}" ]; then
  chown -R "$HOST_UID:${HOST_GID:-$HOST_UID}" /work
fi
